#pragma once

class UpdateState;

class UpdateNode {
public:
	inline virtual ~UpdateNode() {
	}

	virtual void update(UpdateState & state) = 0;
};

