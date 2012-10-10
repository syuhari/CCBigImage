//
//  TestNode.cpp
//  BigImage
//
//  Created by 松浦 晃洋 on 12/10/09.
//
//

#include "TestNode.h"

TestNode* TestNode::nodeWithImageForRect(string anImage)
{
    TestNode* pRet = new TestNode();
    if (pRet->initWithImageForRect(anImage))
    {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

bool TestNode::initWithImageForRect(string anImage)
{
    this->_sprite = new CCSprite;
    this->_sprite->initWithFile(anImage.c_str());
	this->_sprite->setAnchorPoint(ccp(0,0));
	this->_sprite->setPosition(ccp(0,0));
    this->addChild(_sprite);
	return true;
}

void TestNode::visit()
{
    /*
	// quick return if not visible
	if (!this->getIsVisible()) return;
	
    if (!this->_sprite) return;
    
    glPushMatrix();
	
    this->transform();
	this->_sprite->visit();
	
    glPopMatrix();
    */
    CCNode::visit();
}