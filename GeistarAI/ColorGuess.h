#pragma once

#include <string>
#include "types.h"
#include "method.h"

namespace Red {

	extern int eval[16];	//ê‘ìx
	extern Board after_me, after_enemy;

	void init();
	void setAfterMe(string str);
	void setAfterEnemy(string str);
	void setEval();
	void setEvalafterMe();
}