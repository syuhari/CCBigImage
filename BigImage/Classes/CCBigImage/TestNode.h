//
//  TestNode.h
//  BigImage
//
//  Created by 松浦 晃洋 on 12/10/09.
//
//

#ifndef __BigImage__TestNode__
#define __BigImage__TestNode__

#include "cocos2d.h"
using namespace cocos2d;

class TestNode : public CCNode
{
private:
public:
    CC_SYNTHESIZE(CCSprite*, _sprite, Sprite);
    bool initWithImageForRect(string anImage);
    static TestNode* nodeWithImageForRect(string anImage);
    virtual void visit();
};

#endif /* defined(__BigImage__TestNode__) */
