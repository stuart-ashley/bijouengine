#include "shaderFlag.h"
#include "shaderTag.h"

#include "shaderManager.h"

#include "../scripting/executable.h"
#include "../scripting/kwarg.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/string.h"

#include <algorithm>

using namespace render;

namespace {

	struct Factory: public Executable {
		std::string currentDir;

		Factory(const std::string & currentDir) :
				currentDir(currentDir) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto & shaderMan = ShaderManager::getInstance();
			std::shared_ptr<Program> vp;
			std::shared_ptr<Program> gp;
			std::shared_ptr<Program> fp;

			for (unsigned i = 0; i < nArgs; ++i) {
				// pop arg
				auto arg = stack.top();
				stack.pop();

				// assert keyword argument
				scriptExecutionAssertType<Kwarg>(arg,
						"Require keyword argument");

				// keyword name
				const auto & name =
						std::static_pointer_cast<Kwarg>(arg)->getKey();

				// pop val
				auto val = stack.top();
				stack.pop();

				// assert string value
				scriptExecutionAssertType<String>(val,
						"Unacceptable keyword argument '" + name + "'");

				// keyword value
				const auto & value =
						std::static_pointer_cast<String>(val)->getValue();

				if (name == "vp") {
					vp = shaderMan.getProgram(currentDir, value);
				} else if (name == "gp") {
					gp = shaderMan.getProgram(currentDir, value);
				} else if (name == "fp") {
					fp = shaderMan.getProgram(currentDir, value);
				} else {
					scriptExecutionAssert(false,
							"Unknown keyword argument '" + name + "'");
				}

			}
			stack.emplace(std::make_shared<ShaderTag>(vp, gp, fp));
		}
	};

	size_t getHashCode(const std::shared_ptr<Program> & vp,
			const std::shared_ptr<Program> & gp,
			const std::shared_ptr<Program> & fp, const ShaderFlags & flags) {
		size_t result = 17;
		if (vp != nullptr) {
			result = 31 * result + std::hash<std::string>()(vp->getName());
		}
		if (gp != nullptr) {
			result = 31 * result + std::hash<std::string>()(gp->getName());
		}
		if (fp != nullptr) {
			result = 31 * result + std::hash<std::string>()(fp->getName());
		}
		result = 31 * result + std::hash<ShaderFlags>()(flags);
		return result;
	}
}

struct ShaderTag::impl {
	std::shared_ptr<Program> vp;
	std::shared_ptr<Program> gp;
	std::shared_ptr<Program> fp;
	ShaderFlags flags;
	bool valid;
	ShaderFlags validFlags;

	impl(const std::shared_ptr<Program> & vp,
			const std::shared_ptr<Program> & gp,
			const std::shared_ptr<Program> & fp) :
			vp(vp), gp(gp), fp(fp), valid(false) {
	}

	impl(const std::shared_ptr<Program> & vp,
			const std::shared_ptr<Program> & gp,
			const std::shared_ptr<Program> & fp, const ShaderFlags & flags) :
			vp(vp), gp(gp), fp(fp), flags(flags), valid(false) {
	}

	impl(const ShaderTag::impl & other) :
					vp(other.vp),
					gp(other.gp),
					fp(other.fp),
					flags(other.flags),
					valid(other.valid),
					validFlags(other.validFlags) {
	}

	bool operator==(const ShaderTag::impl & other) const {
		if (vp != other.vp) {
			return false;
		}
		if (gp != other.gp) {
			return false;
		}
		if (fp != other.fp) {
			return false;
		}
		if (flags != other.flags) {
			return false;
		}
		return true;
	}
};

/*
 *
 */
ShaderTag::ShaderTag(const std::shared_ptr<Program> & vp,
		const std::shared_ptr<Program> & gp,
		const std::shared_ptr<Program> & fp) :
		pimpl(new impl(vp, gp, fp)) {
}

/*
 *
 */
ShaderTag::ShaderTag(const std::shared_ptr<Program> & vp,
		const std::shared_ptr<Program> & gp,
		const std::shared_ptr<Program> & fp, const ShaderFlags & flags) :
		pimpl(new impl(vp, gp, fp, flags)) {
}

/*
 *
 */
ShaderTag::ShaderTag(const ShaderTag & other) :
		pimpl(new impl(*other.pimpl)) {
}

/*
 *
 */
ShaderTag::~ShaderTag() {
}
/*
 *
 */
void ShaderTag::addFlag(size_t flag) {
	pimpl->flags[flag] = true;
}
/*
 *
 */
void ShaderTag::addFlags(const ShaderFlags & additionalFlags) {
	pimpl->flags |= additionalFlags;
}
/*
 *
 */
ShaderFlags ShaderTag::getActiveFlags() const {
	ShaderFlags validFlags = pimpl->vp->getValidFlags();
	if (pimpl->gp != nullptr) {
		validFlags |= pimpl->gp->getValidFlags();
	}
	validFlags |= pimpl->fp->getValidFlags();

	return pimpl->flags & validFlags;
}

/*
 *
 */
const std::shared_ptr<Program> & ShaderTag::getFragmentProgram() const {
	return pimpl->fp;

}

/*
 *
 */
const std::shared_ptr<Program> & ShaderTag::getGeometryProgram() const {
	return pimpl->gp;

}

/*
 *
 */
size_t ShaderTag::getHash() const {
	return getHashCode(pimpl->vp, pimpl->gp, pimpl->fp, pimpl->flags);

}

/*
 *
 */
const std::shared_ptr<Program> & ShaderTag::getVertexProgram() const {
	return pimpl->vp;
}

/*
 *
 */
bool ShaderTag::hasFlag(size_t flag) const {
	return pimpl->flags[flag];
}

/*
 *
 */
bool ShaderTag::operator==(const ShaderTag & other) const {
	if (pimpl->vp != other.pimpl->vp) {
		return false;
	}
	if (pimpl->gp != other.pimpl->gp) {
		return false;
	}
	if (pimpl->fp != other.pimpl->fp) {
		return false;
	}
	if (pimpl->flags != other.pimpl->flags) {
		return false;
	}
	return true;
}

/*
 *
 */
void ShaderTag::overlay(const ShaderTag & overlay) {
	// flags
	pimpl->flags |= overlay.pimpl->flags;

	// programs
	if (overlay.pimpl->vp != nullptr || overlay.pimpl->gp != nullptr
			|| overlay.pimpl->fp != nullptr) {
		pimpl->vp = overlay.pimpl->vp;
		pimpl->gp = overlay.pimpl->gp;
		pimpl->fp = overlay.pimpl->fp;
	}
}

/*
 *
 */
void ShaderTag::removeFlag(size_t flag) {
	pimpl->flags[flag] = false;
}

/*
 *
 */
std::string ShaderTag::toString() const {
	std::string str = "ShaderTag( vp=";
	if (pimpl->vp == nullptr) {
		str += "NULL";
	} else {
		str += pimpl->vp->getName();
	}
	str += ", gp=";
	if (pimpl->gp == nullptr) {
		str += "NULL";
	} else {
		str += pimpl->gp->getName();
	}
	str += ", fp=";
	if (pimpl->fp == nullptr) {
		str += "NULL";
	} else {
		str += pimpl->fp->getName();
	}
	str += ", flags=[";
	bool first = true;
	for (size_t i = 0, n = pimpl->flags.size(); i < n; ++i) {
		if (pimpl->flags[i] == false) {
			continue;
		}
		if (first == false) {
			str += ", ";
		} else {
			first = false;
		}
		str += ShaderFlag::toString(i);
	}
	str += "] )";
	return str;
}

/*
 *
 */
bool ShaderTag::validate() const {
	if (pimpl->valid) {
		return true;
	}
	pimpl->valid = (pimpl->vp == nullptr || pimpl->vp->isLoaded())
			&& (pimpl->gp == nullptr || pimpl->gp->isLoaded())
			&& (pimpl->fp == nullptr || pimpl->fp->isLoaded());
	return pimpl->valid;
}

/**
 * get script object factory for ShaderTag
 *
 * @param currentDir  current directory of caller
 *
 * @return            ShaderTag factory
 */
STATIC ScriptObjectPtr ShaderTag::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
