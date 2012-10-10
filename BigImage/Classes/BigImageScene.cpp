#include "BigImageScene.h"
#include "SimpleAudioEngine.h"
#include "CCBigImageLayer.h"

using namespace cocos2d;
using namespace CocosDenshion;

#define kLayerTag 1

CCScene* BigImage::scene()
{
	CCScene *scene = CCScene::node();
	BigImage *layer = BigImage::node();
	scene->addChild(layer);

	return scene;
}

bool BigImage::init()
{
	if ( !CCLayer::init() )
	{
		return false;
	}

	CCBigImageLayer* layer = CCBigImageLayer::node();
    this->addChild(layer, kLayerTag, kLayerTag);
    
	return true;
}

void BigImage::onEnterTransitionDidFinish() {
    CCBigImageLayer* layer = (CCBigImageLayer*)this->getChildByTag(kLayerTag);
    layer->loadBigImage("bigImage.plist");
}

