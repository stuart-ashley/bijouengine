#pragma once

#include "updateNode.h"
#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../core/vec3.h"

#include "../scripting/scriptObject.h"

class SgTranslate final: public ScriptObject,
		public UpdateNode,
		public TaskInitNode,
		public VisualizeNode {
public:

	SgTranslate(const Vec3 & translation);

	/**
	 * default destructor
	 */
	inline virtual ~SgTranslate() = default;

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
	 *
	 * @return
	 */
	Vec3 getTranslation() const;

	/**
	 *
	 * @param translation
	 */
	void setTranslation(const Vec3 & translation);

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 *
	 * @param state
	 */
	void update(UpdateState & state) override;

	/**
	 *
	 * @param vb
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgTranslate
	 *
	 * @return  SgTranslate factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	Vec3 m_translation;
};

