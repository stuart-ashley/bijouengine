#pragma once

class Builder;

class TaskInitNode {
public:
	inline virtual ~TaskInitNode() {
	}

	virtual void taskInit(Builder & builder) = 0;
};
