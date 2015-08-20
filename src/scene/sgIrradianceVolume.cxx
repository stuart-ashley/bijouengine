#include "sgIrradianceVolume.h"

#include "builder.h"
#include "lights.h"
#include "taskWrapper.h"

#include "../core/binaryFile.h"
#include "../core/binaryFileCache.h"
#include "../core/boundingBox.h"
#include "../core/color.h"
#include "../core/normal.h"
#include "../core/transform.h"
#include "../core/vec3.h"

#include "../render/irradianceVolume.h"
#include "../render/renderTask.h"
#include "../render/setTexture.h"
#include "../render/texture.h"
#include "../render/textureManager.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/string.h"

#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

using namespace render;

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<String>("file", std::make_shared<String>("")),
			Parameter<Vec3>("extents", std::make_shared<Vec3>(
					std::numeric_limits<double>::max(),
					std::numeric_limits<double>::max(),
					std::numeric_limits<double>::max())),
			Parameter<Vec3>("size", nullptr),
			Parameter<List>("values", std::make_shared<List>()) };

	/*
	 *
	 */
	struct Factory: public Executable {
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		static std::vector<double> getValues(
				std::unordered_map<std::string, ScriptObjectPtr> & args) {
			std::vector<double> values;

			for (const auto & e : *std::static_pointer_cast<List>(
					args["values"])) {
				scriptExecutionAssertType<Real>(e, "Require numeric values");

				values.emplace_back(
						(std::static_pointer_cast<Real>(e))->getValue());
			}

			return values;
		}

		static std::string getUniqueName() {
			static int counter = 0;
			std::string name = "irradiance_volume_" + counter;
			++counter;
			return name;
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);

			const auto & file =
					std::static_pointer_cast<String>(args["file"])->getValue();

			const auto & extents = *std::static_pointer_cast<Vec3>(
					args["extents"]);

			const auto & size = *std::static_pointer_cast<Vec3>(args["size"]);
			auto sx = static_cast<int>(size.getX());
			auto sy = static_cast<int>(size.getY());
			auto sz = static_cast<int>(size.getZ());

			auto list = getValues(args);

			if (file != "") {
				stack.push(
						std::make_shared<SgIrradianceVolume>(file,
								BinaryFileCache::get(currentDir, file), extents,
								sx, sy, sz));
			} else if (list.size() > 0) {
				auto values = getValues(args);
				int vol = sx * sy * sz;

				std::unique_ptr<uint8_t[]> buffer( new uint8_t[ vol * 28 ] );

				for (int i = 0; i < vol; ++i) {
					// maximum absolute value for pixel
					double mav = 0;
					for (int j = 0; j < 27; ++j) {
						mav = std::max(mav, std::abs(values[i * 27 + j]));
					}
					// normalize pixel values
					double exp = (mav == 0) ? 0 : std::ceil(std::log2(mav));
					double scale = 255 / std::pow(2, exp + 1);
					uint8_t bytes[28];
					for (int j = 0; j < 27; ++j) {
						bytes[j] = static_cast<uint8_t>(std::round(
								values[i * 27 + j] * scale) + 127);
					}
					bytes[27] = static_cast<uint8_t>(exp + 127);

					// vol * 0 + i * 4
					buffer[i * 4] = bytes[0];
					buffer[i * 4 + 1] = bytes[3];
					buffer[i * 4 + 2] = bytes[6];
					buffer[i * 4 + 3] = bytes[9];

					// vol * 4 + i * 4
					buffer[(vol + i) * 4] = bytes[1];
					buffer[(vol + i) * 4 + 1] = bytes[4];
					buffer[(vol + i) * 4 + 2] = bytes[7];
					buffer[(vol + i) * 4 + 3] = bytes[10];

					// vol * 8 + i * 4
					buffer[(vol * 2 + i) * 4] = bytes[2];
					buffer[(vol * 2 + i) * 4 + 1] = bytes[5];
					buffer[(vol * 2 + i) * 4 + 2] = bytes[8];
					buffer[(vol * 2 + i) * 4 + 3] = bytes[11];

					// ( vol * 3 + i ) * 4
					buffer[(vol * 3 + i) * 4] = bytes[12];
					buffer[(vol * 3 + i) * 4 + 1] = bytes[13];
					buffer[(vol * 3 + i) * 4 + 2] = bytes[14];
					buffer[(vol * 3 + i) * 4 + 3] = bytes[24];

					buffer[(vol * 4 + i) * 4] = bytes[15];
					buffer[(vol * 4 + i) * 4 + 1] = bytes[16];
					buffer[(vol * 4 + i) * 4 + 2] = bytes[17];
					buffer[(vol * 4 + i) * 4 + 3] = bytes[25];

					buffer[(vol * 5 + i) * 4] = bytes[18];
					buffer[(vol * 5 + i) * 4 + 1] = bytes[19];
					buffer[(vol * 5 + i) * 4 + 2] = bytes[20];
					buffer[(vol * 5 + i) * 4 + 3] = bytes[26];

					buffer[(vol * 6 + i) * 4] = bytes[21];
					buffer[(vol * 6 + i) * 4 + 1] = bytes[22];
					buffer[(vol * 6 + i) * 4 + 2] = bytes[23];
					buffer[(vol * 6 + i) * 4 + 3] = bytes[27];
				}

				stack.push(
						std::make_shared<SgIrradianceVolume>(getUniqueName(),
								buffer, extents, sx, sy, sz));
			} else {
				scriptExecutionAssert(false,
						"Require file or values for SgIrradianceVolume");
			}
		}
	};

	/*
	 *
	 */
	class GetIrradianceSample: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto sgIrradianceVolume = std::static_pointer_cast<
					SgIrradianceVolume>(self);

			auto point = getArg<Vec3>("Vec3", stack, 1);

			auto normal = getArg<Normal>("Normal", stack, 1);

			if (sgIrradianceVolume->getVolume() != nullptr) {
				stack.push(
						std::make_shared<Color>(
								sgIrradianceVolume->getVolume()->getSample(
										point, normal)));
				return;
			}

			stack.push(std::make_shared<Color>(Color::white()));
		}
	};

	std::array<std::string, 7> ext = { { ".0", ".1", ".2", ".3", ".4", ".5",
			".6" } };
}

struct SgIrradianceVolume::impl {
	std::string name;
	std::shared_ptr<BinaryFile> binFile;
	BoundingBox bounds;
	std::array<std::shared_ptr<Texture>, 7> targets;
	std::array<std::shared_ptr<RenderTask>, 7> load;
	int sizeX;
	int sizeY;
	int sizeZ;
	std::unique_ptr<AbstractIrradianceVolume> volume;
	bool loadNeeded;
	std::unique_ptr<uint8_t[]> buffer;

	impl(const std::string & name, const std::shared_ptr<BinaryFile> & binFile,
			const Vec3 & extents, int sizeX, int sizeY, int sizeZ) :
					name(name),
					binFile(binFile),
					bounds(extents * -.5, extents * .5),
					sizeX(sizeX),
					sizeY(sizeY),
					sizeZ(sizeZ),
					loadNeeded(false),
					buffer(nullptr) {
		TextureManager::FlagSet flags;
		flags[TextureManager::Flag::RGBA8] = true;

		for (size_t i = 0, n = ext.size(); i < n; ++i) {
			targets[i] = TextureManager::getInstance().get3dTexture(
					name + ext[i], sizeX, sizeY, sizeZ, flags);
		}
	}

	impl(const std::string & name, std::unique_ptr<uint8_t[]> & buff, const Vec3 & extents,
			int sizeX, int sizeY, int sizeZ) :
					name(name),
					binFile(nullptr),
					bounds(extents * -.5, extents * .5),
					sizeX(sizeX),
					sizeY(sizeY),
					sizeZ(sizeZ),
					loadNeeded(false),
					buffer(std::move(buff)) {
		TextureManager::FlagSet flags;
		flags[TextureManager::Flag::RGBA8] = true;

		for (size_t i = 0, n = ext.size(); i < n; ++i) {
			targets[i] = TextureManager::getInstance().get3dTexture(
					name + ext[i], sizeX, sizeY, sizeZ, flags);
		}

		volume = std::unique_ptr<AbstractIrradianceVolume>(
				new AbstractIrradianceVolume(sizeX, sizeY, sizeZ, buffer.get(),
						targets, bounds));
	}
};

/**
 * constructor
 *
 * @param name
 * @param binFile
 * @param extents
 * @param size
 */
SgIrradianceVolume::SgIrradianceVolume(const std::string & name,
		const std::shared_ptr<BinaryFile> & binFile, const Vec3 & extents,
		int sizeX, int sizeY, int sizeZ) :
		pimpl(new impl(name, binFile, extents, sizeX, sizeY, sizeZ)) {
}

/**
 * constructor
 *
 * @param name
 * @param buffer
 * @param extents
 * @param size
 */
SgIrradianceVolume::SgIrradianceVolume(const std::string & name,
		std::unique_ptr<uint8_t[]> & buffer, const Vec3 & extents, int sizeX, int sizeY,
		int sizeZ) :
		pimpl(new impl(name, buffer, extents, sizeX, sizeY, sizeZ)) {
}

/**
 * destructor
 */
SgIrradianceVolume::~SgIrradianceVolume() {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgIrradianceVolume::getMember(
		ScriptExecutionState & execState, const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = { {
			"getIrradianceSample", std::make_shared<GetIrradianceSample>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 * @return
 */
const std::unique_ptr<render::AbstractIrradianceVolume> & SgIrradianceVolume::getVolume() const {
	return pimpl->volume;
}

/**
 *
 * @param builder
 */
OVERRIDE void SgIrradianceVolume::taskInit(Builder & builder) {
	// load
	if (pimpl->volume == nullptr) {
		if (pimpl->binFile->valid() == false) {
			return;
		}
		pimpl->volume =
				std::unique_ptr<AbstractIrradianceVolume>(
						new AbstractIrradianceVolume(pimpl->sizeX, pimpl->sizeY,
								pimpl->sizeZ,
								reinterpret_cast<const unsigned char *>(pimpl->binFile->getData()),
								pimpl->targets, pimpl->bounds));
	}
	if (pimpl->volume->validate() == false) {
		if (pimpl->loadNeeded == false) {
			pimpl->loadNeeded = true;

			auto ptr = reinterpret_cast<const char *>(pimpl->volume->getBuffer());

			for (size_t i = 0; i < pimpl->targets.size(); ++i) {
				int nPixels = pimpl->targets[i]->getWidth()
						* pimpl->targets[i]->getHeight()
						* pimpl->targets[i]->getDepth();

				pimpl->load[i] = std::make_shared<SetTexture>(
						pimpl->name + ".load." + std::to_string(i),
						pimpl->targets[i], ptr);

				ptr += nPixels * 4;
			}
		}
	}
	if (pimpl->loadNeeded) {
		pimpl->loadNeeded = false;
		for (size_t i = 0; i < pimpl->targets.size(); ++i) {
			if (pimpl->load[i] == nullptr) {
				continue;
			}
			if (pimpl->load[i]->hasExecuted()) {
				pimpl->load[i] = nullptr;
			} else {
				builder.addTask(std::make_shared<TaskWrapper>(pimpl->load[i]));
				pimpl->loadNeeded = true;
			}
		}
	}
	auto transform = builder.getTransform();
	builder.getLights().addIrradianceVolume(
			IrradianceVolume(*pimpl->volume, transform));
}

/**
 * get script object factory for SgIrradianceVolume
 *
 * @param currentDir  current directory of caller
 *
 * @return            SgIrradianceVolume factory
 */
STATIC ScriptObjectPtr SgIrradianceVolume::getFactory(
		const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
