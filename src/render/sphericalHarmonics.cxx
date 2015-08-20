#include "sphericalHarmonics.h"

#include "panel.h"
#include "readBack.h"
#include "shaderManager.h"
#include "textureManager.h"
#include "uniform.h"
#include "uniformArray.h"

#include "../core/color.h"
#include "../core/rect.h"

#include <cassert>

using namespace render;

namespace {

	auto SHVectorsUID = Uniform::getUID("SHVectors");
	auto SHCoefficientsUID = Uniform::getUID("SHCoefficients");
	auto CubeMapUID = Uniform::getUID("CubeMap");

	Rect rect(0, 1, 0, 1);

	std::shared_ptr<Texture> projVectors;
	std::shared_ptr<Texture> projCoeffs[3];
	std::shared_ptr<Shader> projShaders[3];
}

struct SphericalHarmonics::impl {
	std::unique_ptr<ReadBack> readBacks[9];
	std::unique_ptr<Panel> panels[9];
	std::array<float, 27> coefficients;
	bool executed;

	impl(const std::array<float, 27> & coefficients) :
			coefficients(coefficients), executed(false) {
	}
};

/**
 *
 * @param name
 * @param texture
 * @param coefficients
 */
SphericalHarmonics::SphericalHarmonics(const std::string & name,
		const std::shared_ptr<Texture> & texture,
		const std::array<float, 27> & coefficients) :
				RenderTask(name, nullptr, Rect(0, 1, 0, 1), 0),
				pimpl(new impl(coefficients)) {
	addDependency(texture->getId());

	for (int i = 0; i < 9; ++i) {
		const auto & shader = projShaders[i % 3];

		UniformArray uniforms;
		uniforms.add(Uniform(SHVectorsUID, projVectors));
		uniforms.add(Uniform(SHCoefficientsUID, projCoeffs[i / 3]));
		uniforms.add(Uniform(CubeMapUID, texture));

		auto tex = TextureManager::getInstance().getTexture(
				name + " tex" + std::to_string(i), 1, 1, false);

		pimpl->panels[i] = std::unique_ptr<Panel>(
				new Panel("", rect, rect, uniforms, shader, tex, projVectors,
						BlendFlag::NONE, 0, Color::white(), 1, false,
						Color::black()));
		pimpl->readBacks[i] = std::unique_ptr<ReadBack>(
				new ReadBack(tex, 0, 0, 1, 1));
	}
}

/**
 *
 */
OVERRIDE void SphericalHarmonics::execute(Canvas & canvas) {
	for (int i = 0; i < 9; ++i) {
		pimpl->panels[i]->execute(canvas);
	}

	for (int i = 0; i < 9; ++i) {
		pimpl->readBacks[i]->execute(canvas);
	}

	for (int i = 0; i < 9; ++i) {
		const auto & bb = pimpl->readBacks[i]->getBytes();
		pimpl->coefficients[i * 3] = static_cast<float>(bb[0] - 128) / 128.f;
		pimpl->coefficients[i * 3 + 1] = static_cast<float>(bb[1] - 128)
				/ 128.f;
		pimpl->coefficients[i * 3 + 2] = static_cast<float>(bb[2] - 128)
				/ 128.f;
	}

	pimpl->executed = true;
}

/**
 *
 * @return
 */
OVERRIDE bool SphericalHarmonics::hasExecuted() const {
	return pimpl->executed;
}

/**
 *
 */
STATIC void SphericalHarmonics::init() {
	auto & texMan = TextureManager::getInstance();
	auto & shaderMan = ShaderManager::getInstance();

	projVectors = texMan.getImage("textures/", "SHVec.png", false, true, false);
	projCoeffs[0] = texMan.getImage("textures/", "SHCoeff0.png", false, true,
			false);
	projCoeffs[1] = texMan.getImage("textures/", "SHCoeff1.png", false, true,
			false);
	projCoeffs[2] = texMan.getImage("textures/", "SHCoeff2.png", false, true,
			false);

	auto vp = shaderMan.getProgram("shaders/", "colmap.vp");
	auto fpx = shaderMan.getProgram("shaders/", "shx.fp");
	auto fpy = shaderMan.getProgram("shaders/", "shy.fp");
	auto fpz = shaderMan.getProgram("shaders/", "shz.fp");

	projShaders[0] = shaderMan.getShader(ShaderTag(vp, nullptr, fpx));
	projShaders[1] = shaderMan.getShader(ShaderTag(vp, nullptr, fpy));
	projShaders[2] = shaderMan.getShader(ShaderTag(vp, nullptr, fpz));
}
