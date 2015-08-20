#pragma once

#include "visualizeNode.h"

#include "../render/uniform.h"

#include "../scripting/scriptObject.h"

namespace render {
	class ViewBuilder;
}

class SgUniform final: public ScriptObject, public VisualizeNode {
public:

	/**
	 * Construct from uniform
	 *
	 * @param uniform
	 *            Uniform to wrap
	 */
	SgUniform(const render::Uniform & uniform);

	/**
	 * default destructor
	 */
	inline virtual ~SgUniform() = default;

	/**
	 * get uniform id
	 *
	 * @return  uniform id
	 */
	size_t getId() const;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const override;

	/**
	 * set uniform value
	 *
	 * @param uniform  uniform value
	 */
	void set(const render::Uniform & uniform);

	/**
	 * Add uniform to render state
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgUniform
	 *
	 * @return  SgUniform factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	render::Uniform m_uniform;
};

