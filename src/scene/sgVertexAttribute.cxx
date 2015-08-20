#include "sgVertexAttribute.h"

#include "../core/color.h"
#include "../core/normal.h"
#include "../core/vec3.h"

#include "../render/renderState.h"
#include "../render/vertexBuffer.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <cassert>

using namespace render;

namespace {
	std::vector<BaseParameter> params = { Parameter<VertexBuffer>("buffer",
			nullptr), Parameter<String>("label", nullptr), Parameter<Real>(
			"size", nullptr), Parameter<String>("type", nullptr),
			Parameter<Real>("offset", nullptr), Parameter<Real>("stride",
					nullptr) };
	/*
	 *
	 */
	struct Factory: public Executable {
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {

			if (typeid(*stack.top()) == typeid(String)) {
				const auto & label = std::static_pointer_cast<String>(
						stack.top())->getValue();
				stack.pop();

				auto list = getArg<List>("list", stack, 2);

				if (typeid(*list.get(0)) == typeid(Vec3)) {
					std::vector<Vec3> vertices;
					vertices.reserve(list.size());

					for (const auto & e : list) {
						scriptExecutionAssertType<Vec3>(e,
								"List needs to be of one type");
						vertices.emplace_back(
								*std::static_pointer_cast<Vec3>(e));
					}
					stack.push(
							std::make_shared<SgVertexAttribute>(
									VertexAttribute(label, vertices)));
					return;
				} else if (typeid(*list.get(0)) == typeid(Normal)) {
					std::vector<Normal> normals;
					normals.reserve(list.size());

					for (const auto & e : list) {
						scriptExecutionAssertType<Normal>(e,
								"List needs to be of one type");
						normals.emplace_back(
								*std::static_pointer_cast<Normal>(e));
					}
					stack.push(
							std::make_shared<SgVertexAttribute>(
									VertexAttribute(label, normals)));
					return;
				} else if (typeid(*list.get(0)) == typeid(Color)) {
					std::vector<Color> colors;
					colors.reserve(list.size());

					for (const auto & e : list) {
						scriptExecutionAssertType<Color>(e,
								"List needs to be of one type");
						colors.emplace_back(
								*std::static_pointer_cast<Color>(e));
					}
					stack.push(
							std::make_shared<SgVertexAttribute>(
									VertexAttribute(label, colors)));
					return;
				} else {
					scriptExecutionAssert(false,
							"Need list of Vec3, Normal or Color");
				}
			}
			auto args = parameters.getArgs(nArgs, stack);

			auto buffer = std::static_pointer_cast<VertexBuffer>(
					args["buffer"]);
			auto label =
					std::static_pointer_cast<String>(args["label"])->getValue();
			int size = std::static_pointer_cast<Real>(args["size"])->getInt32();
			auto typeStr =
					std::static_pointer_cast<String>(args["type"])->getValue();
			VertexAttribute::Type type;
			if (typeStr == "BYTE") {
				type = VertexAttribute::Type::BYTE;
			} else if (typeStr == "FLOAT") {
				type = VertexAttribute::Type::FLOAT;
			} else if (typeStr == "SHORT") {
				type = VertexAttribute::Type::SHORT;
			} else if (typeStr == "USHORT") {
				type = VertexAttribute::Type::USHORT;
			} else {
				assert(false);
			}
			int offset =
					std::static_pointer_cast<Real>(args["offset"])->getInt32();
			int stride =
					std::static_pointer_cast<Real>(args["stride"])->getInt32();
			stack.push(
					std::make_shared<SgVertexAttribute>(
							VertexAttribute(buffer, label, size, type, offset,
									stride)));
		}
	};

}

/**
 *
 * @param attribute
 */
SgVertexAttribute::SgVertexAttribute(const render::VertexAttribute & attribute) :
		m_attribute(attribute) {
}

/**
 *
 * @param vb
 */
OVERRIDE void SgVertexAttribute::visualize(render::ViewBuilder & vb) {
	vb.getState().addVertexAttribute(m_attribute);
}

/**
 * get script object factory for SgVertexAttribute
 *
 * @param currentDir  current directory of caller
 *
 * @return            SgVertexAttribute factory
 */
STATIC ScriptObjectPtr SgVertexAttribute::getFactory(
		const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
