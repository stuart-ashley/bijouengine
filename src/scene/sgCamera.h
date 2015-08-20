#pragma once

#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

#include <memory>

class Aspect;
class PerspectiveCamera;

namespace render {
	class ViewBuilder;
}

class SgCamera final: public ScriptObject, public TaskInitNode, public VisualizeNode {
public:

	/**
	 *
	 * @param cam
	 */
	SgCamera(const std::shared_ptr<PerspectiveCamera> & camera);

	/**
	 * default destructor
	 */
	inline virtual ~SgCamera() = default;

	/**
	 *
	 * @return
	 */
	const std::shared_ptr<Aspect> & getAspect() const;

	/**
	 *
	 * @return
	 */
	const std::shared_ptr<PerspectiveCamera> & getCamera() const;

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
	 * @param camera
	 */
	void setCamera(const std::shared_ptr<PerspectiveCamera> & camera);

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 *
	 * @param vb
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgCamera
	 *
	 * @return  SgCamera factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	std::shared_ptr<PerspectiveCamera> m_camera;
	std::shared_ptr<Aspect> m_aspect;
};

