#include "rect.h"
#include "vec2.h"

#include "../scripting/bool.h"
#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <unordered_map>

namespace {

	/*
	 *
	 */
	class Factory: public Executable {
	public:
		Factory() {
		}

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 4);

			auto l = getFloatArg(stack, 1);
			auto r = getFloatArg(stack, 2);
			auto b = getFloatArg(stack, 3);
			auto t = getFloatArg(stack, 4);

			scriptExecutionAssert(l < r, "right must be greater than left");

			scriptExecutionAssert(b < t, "top must be greater than bottom");

			stack.emplace(std::make_shared<Rect>(l, r, b, t));
		}
	};

	/*
	 *
	 */
	class GetWidth: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			double width = std::static_pointer_cast<Rect>(self)->getWidth();
			stack.emplace(std::make_shared<Real>(width));
		}
	};

	/*
	 *
	 */
	class GetHeight: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			double height = std::static_pointer_cast<Rect>(self)->getHeight();
			stack.emplace(std::make_shared<Real>(height));
		}
	};

	/*
	 *
	 */
	class Contains: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			const auto & rect = std::static_pointer_cast<Rect>(self);

			auto point = getArg<Vec2>("Vec2", stack, 1);

			bool contains = rect->contains(point);

			if (contains) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	/*
	 *
	 */
	class Intersects: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			const auto & rect = std::static_pointer_cast<Rect>(self);

			auto other = getArg<Rect>("Rect", stack, 1);

			bool intersects = rect->intersects(other);

			if (intersects) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	/*
	 *
	 */
	bool check(float left, float right, float bottom, float top) {
		if (left >= right) {
			std::cerr << "left:" << left << " >= right:" << right << std::endl;
			return false;
		}
		if (bottom >= top) {
			std::cerr << "bottom:" << bottom << " >= top:" << top << std::endl;
			return false;
		}
		return true;
	}
}

/**
 * constructor
 *
 * @param left
 * @param right
 * @param bottom
 * @param top
 */
Rect::Rect(float left, float right, float bottom, float top) :
		m_left(left), m_right(right), m_bottom(bottom), m_top(top) {
	assert(check(left, right, bottom, top));
}

/**
 * destructor
 */
Rect::~Rect() {
}

/*
 *
 */
void Rect::set(float left, float right, float bottom, float top) {
	assert(check(left, right, bottom, top));

	m_left = left;
	m_right = right;
	m_bottom = bottom;
	m_top = top;
}

/*
 *
 */
bool Rect::contains(const Vec2 & point) const {
	return m_left < point.getX() && m_right > point.getX()
			&& m_bottom < point.getY() && m_top > point.getY();
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr Rect::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "getWidth", std::make_shared<GetWidth>() },
			{ "getHeight", std::make_shared<GetHeight>() },
			{ "contains", std::make_shared<Contains>() },
			{ "intersects", std::make_shared<Intersects>() } };

	if (name == "left") {
		return std::make_shared<Real>(m_left);
	} else if (name == "right") {
		return std::make_shared<Real>(m_right);
	} else if (name == "bottom") {
		return std::make_shared<Real>(m_bottom);
	} else if (name == "top") {
		return std::make_shared<Real>(m_top);
	}
	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/*
 *
 */
OVERRIDE void Rect::setMember(const std::string & name,
		const ScriptObjectPtr & value) {
	scriptExecutionAssertType<Real>(value, "Require numeric value");

	float x = std::static_pointer_cast<Real>(value)->getFloat();
	if (name == "left") {
		m_left = x;
	} else if (name == "right") {
		m_right = x;
	} else if (name == "bottom") {
		m_bottom = x;
	} else if (name == "top") {
		m_top = x;
	} else {
		scriptExecutionAssert(false, "Can't set member '" + name + "'");
	}
}

/*
 *
 */
OVERRIDE std::string Rect::toString() const {
	return "Rect( left=" + std::to_string(m_left) + ", right="
			+ std::to_string(m_right) + ", bottom=" + std::to_string(m_bottom)
			+ ", top=" + std::to_string(m_top) + " )";
}

/**
 * get script object factory for Rect
 *
 * @return  Rect factory
 */
STATIC const ScriptObjectPtr & Rect::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
