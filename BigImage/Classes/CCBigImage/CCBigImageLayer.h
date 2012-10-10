//
//  CCBigImageLayer.h
//
//  Created by Akihiro Matsuura on 12/09/27.
//
//

#ifndef __CCBigImageLayer__
#define __CCBigImageLayer__

#include "cocos2d.h"
#include "CCBigImage.h"

using namespace cocos2d;

class CCBigImageLayer : public cocos2d::CCLayer
{
private:
    CCPoint startPoint;
    float initialDistance;
    void fixPosition();
    float min_scale;
public:
    virtual ~CCBigImageLayer();
    bool init();
    void loadBigImage(string plist);
    CCBigImage* getBigImage();
    void updateForScreenReshape();

    virtual void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent);
	virtual void ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent);
    
    LAYER_NODE_FUNC(CCBigImageLayer);
};

#endif
