#pragma once

#include "updateNode.h"
#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../core/quat.h"
#include "../core/vec3.h"

#include "../scripting/scriptObject.h"

class SgTransform final: public ScriptObject,
		public UpdateNode,
		public TaskInitNode,
		public VisualizeNode {
public:

	/**
	 * constructor
	 *
	 * @param pitch
	 * @param yaw
	 * @param translation
	 */
	SgTransform(float p, float y, const Vec3 & t);

	/**
	 * default destructor
	 */
	inline virtual ~SgTransform() = default;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const;

	inline float getPitch() const {
		return m_pitch;
	}

	inline const Quat & getRotation() const {
		return m_rotation;
	}

	inline const Vec3 & getTranslation() const {
		return m_translation;
	}

	inline float getYaw() const {
		return m_yaw;
	}

	inline void setPitchYaw(float pitch, float yaw) {
		m_pitch = pitch;
		m_yaw = yaw;
		recalculateRotation();
	}

	inline void setTranslation(const Vec3 & translation) {
		m_translation = translation;
	}

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	inline void translate(const Vec3 & translation) {
		m_translation += translation;
	}

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
	 * get script object factory for SgTransform
	 *
	 * @return  SgTransform factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	/**
	 *
	 */
	void recalculateRotation();

	float m_pitch;
	float m_yaw;
	Vec3 m_translation;
	Quat m_rotation;
};

