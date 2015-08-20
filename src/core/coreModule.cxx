#include "coreModule.h"

#include "animation.h"
#include "bezier.h"
#include "binary.h"
#include "bone.h"
#include "boundingBox.h"
#include "collisionHierarchy.h"
#include "color.h"
#include "convexHull.h"
#include "indexArray.h"
#include "mat4.h"
#include "normal.h"
#include "normalArray.h"
#include "quat.h"
#include "ray.h"
#include "rect.h"
#include "skinningMatrix.h"
#include "sphere.h"
#include "terrain.h"
#include "transform.h"
#include "vec2.h"
#include "vec3.h"
#include "vec3Array.h"

#include <unordered_map>

/*
 *
 */
struct CoreModule::impl {
	std::unordered_map<std::string, ScriptObjectPtr> members;

	impl(const std::string & currentDir) {
		members.emplace("AnimatedUniform", Animation::getFactory());
		members.emplace("Animation", Animation::getFactory());
		members.emplace("Bone", Bone::getFactory());
		members.emplace("Bezier",  Binary::getFactory(currentDir));
		members.emplace("BoundingBox", BoundingBox::getFactory());
		members.emplace("Collision", CollisionHierarchy::getFactory());
		members.emplace("Color", Color::getFactory());
		members.emplace("ConvexHull", ConvexHull::getFactory());
		members.emplace("IndexArray", IndexArray::getFactory(currentDir));
//		members.emplace("Mat3", Mat3::getFactory());
		members.emplace("Mat4", Mat4::getFactory());
		members.emplace("Normal", Normal::getFactory());
		members.emplace("NormalArray",  NormalArray::getFactory(currentDir));
		members.emplace("Quat", Quat::getFactory());
		members.emplace("Ray", Ray::getFactory());
		members.emplace("Rect", Rect::getFactory());
		members.emplace("SkinningMatrix", SkinningMatrix::getFactory());
		members.emplace("Sphere", Sphere::getFactory());
		members.emplace("Terrain", Terrain::getFactory());
		members.emplace("Transform", Transform::getFactory());
		members.emplace("Vec2", Vec2::getFactory());
		members.emplace("Vec3", Vec3::getFactory());
		members.emplace("Vec3Array",  Vec3Array::getFactory(currentDir));
	}
};

/*
 *
 */
CoreModule::CoreModule(const std::string & currentDir) :
		pimpl(new impl(currentDir)) {
}

/*
 *
 */
CoreModule::~CoreModule() {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr CoreModule::getMember(ScriptExecutionState &,
		const std::string & name) const {
	auto entry = pimpl->members.find(name);
	if (entry != pimpl->members.end()) {
		return entry->second;
	}
	return nullptr;
}
