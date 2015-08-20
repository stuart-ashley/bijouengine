#pragma once

#include "normal.h"
#include "vec3.h"

#include "../scripting/scriptObject.h"

class Transform;

class Ray final: public ScriptObject {
public:

	Ray(const Ray &) = default;

	inline Ray(const Vec3 & start, const Vec3 & end) :
					m_start(start),
					m_end(end),
					m_direction((end - start).normalized()),
					m_length((end - start).length()) {
	}

	/**
	 * default destructor
	 */
	inline virtual ~Ray() = default;

	inline const Normal & getDirection() const {
		return m_direction;
	}

	inline const Vec3 & getEnd() const {
		return m_end;
	}

	inline const Vec3 & getStart() const {
		return m_start;
	}

	inline std::string toString() const override {
		return "Ray(" + m_start.toString() + "," + m_end.toString() + ")";
	}

	void transform(const Transform & rotTrans);

	/**
	 * get script object factory for Ray
	 *
	 * @return  Ray factory
	 */
	static ScriptObjectPtr getFactory();

private:
	Vec3 m_start;
	Vec3 m_end;
	Normal m_direction;
	double m_length;
};

