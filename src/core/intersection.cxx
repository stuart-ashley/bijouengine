#include "intersection.h"

#include "transform.h"

void Intersection::transform(const Transform & rotTrans) {
	rotTrans.transformPoint(m_point);
	rotTrans.rotate(m_normal);
}
