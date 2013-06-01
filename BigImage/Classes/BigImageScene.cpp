#include "BigImageScene.h"
#include "SimpleAudioEngine.h"
#include "CCBigImageLayer.h"

using namespace cocos2d;
using namespace CocosDenshion;

#define kLayerTag 1

CCScene* BigImage::scene()
{
	CCScene *scene = CCScene::create();
	BigImage *layer = BigImage::create();
	scene->addChild(layer);

	return scene;
}

bool BigImage::init()
{
	if ( !CCLayer::init() )
	{
		return false;
	}

	CCBigImageLayer* layer = CCBigImageLayer::create();
    this->addChild(layer, kLayerTag, kLayerTag);
    
	return true;
}

void BigImage::onEnterTransitionDidFinish() {
    CCBigImageLayer* layer = (CCBigImageLayer*)this->getChildByTag(kLayerTag);
    layer->loadBigImage("bigImage.plist");
}

