//
//  CCBigImageLayer.cpp
//
//  Created by Akihiro Matsuura on 12/09/27.
//
//

#include "CCBigImageLayer.h"
#include "TestNode.h"

#define MIN_SCALE 0.3
#define MAX_SCALE 1.0
#define INIT_SCALE 0.5

using namespace cocos2d;

enum nodeTags {
	kBigNode,
};

bool CCBigImageLayer::init() {
    if (CCLayer::init() ) {
        this->setIsTouchEnabled(true);
        CCTouchDispatcher::sharedDispatcher()->setDispatchEvents(true);
        
        return true;
    } else {
        return false;
    }
}

void CCBigImageLayer::loadBigImage(string plist) {
    // Create DynamicTiledNode with screen bounds additional preload zone size
    CCBigImage *node = CCBigImage::nodeWithTilesFileTilesExtensionTilesZ(plist, "", 0);
    
    // size of bigImage.png in points
    this->setContentSize(node->getContentSize());
    
    // calc min scale
    float w_scale = CCDirector::sharedDirector()->getWinSize().width / node->getContentSize().width;
    float h_scale = CCDirector::sharedDirector()->getWinSize().height / node->getContentSize().height;
    min_scale = MIN(w_scale, h_scale);
    min_scale = MIN(min_scale, MIN_SCALE);
    
    // Add node as child.
    node->setScale(MIN(INIT_SCALE, min_scale));
    this->addChild(node, 0, kBigNode);
    
    this->updateForScreenReshape();
}

CCBigImage* CCBigImageLayer::getBigImage() {
	CCBigImage *node = (CCBigImage*)this->getChildByTag(kBigNode);
    return node;
}


CCBigImageLayer::~CCBigImageLayer() {
}

void CCBigImageLayer::updateForScreenReshape()
{
	CCSize s = CCDirector::sharedDirector()->getWinSize();
	CCNode *node = this->getChildByTag(kBigNode);
	node->setAnchorPoint(ccp(0.5f, 0.5f));
	node->setPosition(ccp(0.5f * s.width, 0.5f * s.height));
}


#pragma mark Scrolling

void CCBigImageLayer::ccTouchesBegan(CCSet* touches, CCEvent* event) {
    if (touches->count() == 2) {
		// Get points of both touches
        CCSetIterator it = touches->begin();
        CCTouch* tOne = (CCTouch*)(*it);
        CCTouch* tTwo = (CCTouch*)(*++it);
        CCPoint firstTouch = tOne->locationInView(tOne->view());
		CCPoint secondTouch = tTwo->locationInView(tTwo->view());
        
		// Find the distance between those two points
		initialDistance = sqrt(pow(firstTouch.x - secondTouch.x, 2.0f) + pow(firstTouch.y - secondTouch.y, 2.0f));
	}
}

void CCBigImageLayer::ccTouchesMoved(CCSet* touches, CCEvent* event) {
    CCBigImage* node = (CCBigImage*)this->getChildByTag(kBigNode);

    if (touches->count()==1) {
        CCTouch* touch = (CCTouch*)touches->anyObject();
        
        CCSize winSize = CCDirector::sharedDirector()->getWinSize();
        CCRect boundaryRect = CCRectMake(0, 0, winSize.width, winSize.height);
        
        // scrolling is allowed only with non-zero boundaryRect
        if (boundaryRect.size.width>0 && boundaryRect.size.height>0)
        {
            // get touch move delta
            CCPoint point = touch->locationInView(touch->view());
            CCPoint prevPoint = touch->previousLocationInView(touch->view());
            point =  CCDirector::sharedDirector()->convertToGL(point);
            prevPoint = CCDirector::sharedDirector()->convertToGL(prevPoint);
            
            CCPoint delta = ccpSub(point, prevPoint);
            CCPoint newPosition = ccpAdd(node->getPosition(), delta);
            
            CCSize size = node->getContentSize();
            size.width *= node->getScale();
            size.height *= node->getScale();
            
            if (newPosition.x+size.width/2 < winSize.width) {
                newPosition.x = winSize.width - size.width/2;
            } else if (newPosition.x-size.width/2 > 0.0f) {
                newPosition.x = size.width/2;
            }
            
            if (newPosition.y+size.height/2 < winSize.height) {
                newPosition.y = winSize.height - size.height/2;
            } else if (newPosition.y-size.height/2 > 0.0f) {
                newPosition.y = size.height/2;
            }
            
            node->setPosition(newPosition);
            this->fixPosition();
        }
    }
    else if (touches->count() == 2) {
        CCSetIterator it = touches->begin();
        CCTouch* tOne = (CCTouch*)(*it);
        CCTouch* tTwo = (CCTouch*)(*++it);
        CCPoint firstTouch = tOne->locationInView(tOne->view());
		CCPoint secondTouch = tTwo->locationInView(tTwo->view());
		float currentDistance = sqrt(pow(firstTouch.x - secondTouch.x, 2.0f) + pow(firstTouch.y - secondTouch.y, 2.0f));
        
        float zoomFactor = node->getScale();
        float prevScale = node->getScale();

		if (initialDistance == 0) {
			initialDistance = currentDistance;
            // set to 0 in case the two touches weren't at the same time
		} else if (currentDistance - initialDistance > 0) {
			// zoom in
			if (node->getScale() < MAX_SCALE) {
				zoomFactor += zoomFactor *0.05f;
				node->setScale(zoomFactor);
			}
            
			initialDistance = currentDistance;
		} else if (currentDistance - initialDistance < 0) {
			// zoom out
			if (node->getScale() > min_scale) {
				zoomFactor -= zoomFactor *0.05f;
				node->setScale(zoomFactor);
			}
            
			initialDistance = currentDistance;
        }
        
        // update center position
        firstTouch =  CCDirector::sharedDirector()->convertToGL(firstTouch);
        secondTouch =  CCDirector::sharedDirector()->convertToGL(secondTouch);
        CCPoint curPosLayer = ccpMidpoint(firstTouch, secondTouch);

        if (node->getScale() != prevScale)
        {
            CCPoint realCurPosLayer = node->convertToNodeSpace(curPosLayer);
            float deltaX = (realCurPosLayer.x - node->getAnchorPoint().x * node->getContentSize().width) * (node->getScale() - prevScale);
            float deltaY = (realCurPosLayer.y - node->getAnchorPoint().y * node->getContentSize().height)* (node->getScale() - prevScale);
            node->setPosition(ccp(node->getPositionX() - deltaX, node->getPositionY() - deltaY));
        }
	}
}

void CCBigImageLayer::ccTouchesEnded(CCSet* touches, CCEvent* event) {
    this->fixPosition();
}

void CCBigImageLayer::ccTouchesCancelled(CCSet* touches, CCEvent* event) {
    this->fixPosition();
}

void CCBigImageLayer::fixPosition()
{
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    CCRect boundaryRect = CCRectMake(0, 0, winSize.width, winSize.height);
	
    if (boundaryRect.size.width<=0 && boundaryRect.size.height<=0) {
		return;
    }
    
    CCBigImage* node = (CCBigImage*)this->getChildByTag(kBigNode);
    CCSize size = node->getContentSize();
    size.width *= node->getScale();
    size.height *= node->getScale();
    if (size.width < winSize.width ||
        node->getPositionX()-size.width/2 > 0.0f ||
        node->getPositionX()+size.width/2 < winSize.height) {
        node->setPositionX(winSize.width/2);
    }
    
    if (size.height < winSize.height ||
        node->getPositionY()-size.height/2 > 0.0f ||
        node->getPositionY()+size.height/2<winSize.height) {
        node->setPositionY(winSize.height/2);
    }
}