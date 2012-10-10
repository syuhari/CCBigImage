#ifndef __BigImage_SCENE_H__
#define __BigImage_SCENE_H__

#include "cocos2d.h"

class BigImage : public cocos2d::CCLayer
{
public:
	virtual bool init();  
	static cocos2d::CCScene* scene();
    virtual void onEnterTransitionDidFinish();
	LAYER_NODE_FUNC(BigImage);
};

#endif // __BigImage_SCENE_H__
