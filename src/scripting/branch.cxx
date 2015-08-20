#include "branch.h"

#include <string>

struct Branch::impl {
	Type m_type;
	int m_offset;

	/*
	 */
	impl(Type type, int position) :
			m_type(type), m_offset(position) {
	}

	/*
	 */
	impl(const impl & other) :
			m_type(other.m_type), m_offset(other.m_offset) {
	}

};

/*
 */
Branch::Branch(Type type, int offset) :
		pimpl(new impl(type, offset)) {
}

/*
 */
Branch::~Branch() {
}

/*
 */
Branch::Type Branch::getType() const {
	return pimpl->m_type;
}

/*
 */
bool Branch::isType(Branch::Type type) const {
	return pimpl->m_type == type;
}

/*
 */
int Branch::getOffset() const {
	return pimpl->m_offset;
}

