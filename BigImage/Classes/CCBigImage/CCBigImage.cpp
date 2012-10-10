//
//  CCBigImage.cpp
//
//  Created by Akihiro Matsuura on 12/09/27.
//
//

#include "CCBigImage.h"
#include "CCThread.h"
#include "CCNS.h"

UnloadableSpriteNode* UnloadableSpriteNode::nodeWithImageForRect(string anImage, CCRect aRect)
{
    UnloadableSpriteNode* pRet = new UnloadableSpriteNode();
    if (pRet->initWithImageForRect(anImage, aRect))
    {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

bool UnloadableSpriteNode::initWithImageForRect(string anImage, CCRect aRect)
{
    _imageName = anImage;
    _activeRect = aRect;
    setAnchorPoint(ccp(0,0));
    setPosition(aRect.origin);
	return true;
}

#pragma mark CocosNode


// small visit for only one sprite
void UnloadableSpriteNode::visit()
{
	// quick return if not visible
	if (!this->getIsVisible()) return;
	
    if (!this->_sprite) return;
    
    glPushMatrix();
	
    this->transform();
	this->_sprite->visit();
	
    glPopMatrix();
}

CCRect UnloadableSpriteNode::boundingBox()
{
	return _activeRect;
}

UnloadableSpriteNode::~UnloadableSpriteNode()
{
	if (this->_sprite) {
        this->_sprite->release();
        this->_sprite = NULL;
    }
}

#pragma mark Load/Unload

void UnloadableSpriteNode::loadedTexture(CCObject* obj)
{
    CCTexture2D* aTex = (CCTexture2D*)obj;
	aTex->setAntiAliasTexParameters();
	
	//create sprite, position it and at to self
    this->_sprite = new CCSprite;
    this->_sprite->initWithTexture(aTex);
	this->_sprite->setAnchorPoint(ccp(0,0));
	this->_sprite->setPosition(ccp(0,0));
	
	// fill our activeRect fully with sprite (stretch if needed)
	this->_sprite->setScaleX(this->_activeRect.size.width / this->_sprite->getContentSize().width);
	this->_sprite->setScaleY(this->_activeRect.size.height / this->_sprite->getContentSize().height);
    loading = false;
    
    //CCLog("texure 222=%s", CCTextureCache::sharedTextureCache()->description());
}

void UnloadableSpriteNode::unload()
{
    if (this->_sprite) {
        this->_sprite->release();
        this->_sprite = NULL;
    }
}


void UnloadableSpriteNode::load(CCTextureCache* textureCache)
{
	if (this->_sprite || loading) {
		return; //< already loaded or now loading
	}
    
    // load image in other thread
    loading = true;
    textureCache->addImageAsync(_imageName.c_str(), this, callfuncO_selector(UnloadableSpriteNode::loadedTexture));
}

void CCBigImage::setPosition(const CCPoint &newPosition)
{
	float significantPositionDelta = MIN(this->_screenLoadRectExtension.width,
										   this->_screenLoadRectExtension.height) / 2.0f;
	
	if ( ccpLength(ccpSub(newPosition, this->getPosition())) > significantPositionDelta ) {
		this->_significantPositionChange = true;
    }
    
    CCNode::setPosition(newPosition);
}

#pragma mark Init / Creation

CCBigImage* CCBigImage::nodeWithTilesFileTilesExtensionTilesZ(string filename, string extension, int tilesZ)
{
    CCBigImage* pRet = new CCBigImage();
    if (pRet->initWithTilesFileTilesExtensionTilesZ(filename, extension, tilesZ))
    {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return NULL;
}

// designated initializer
bool CCBigImage::initWithTilesFileTilesExtensionTilesZ(string filename, string extension, int tilesZ)
{
    // avoid flickering issue
    CCDirector::sharedDirector()->setDepthTest(false);

    _loadedRect = CCRectZero;
    _screenLoadRectExtension = CCSizeZero;
    _textureCache = CCTextureCache::sharedTextureCache();
    
    string path = CCFileUtils::fullPathFromRelativePath(filename.c_str());
    this->prepareTilesWithFileExtensionZ(path , extension, tilesZ);
	
	return true;
}

// loads info about all tiles,sets self.contentSize & screenLoadRectExtension
// creates & adds tiles for dynamic usage if batchNode
void CCBigImage::prepareTilesWithFileExtensionZ(string plistFile, string extension, int tilesZ)
{
	// load plist with image & tiles info
    CCDictionary<std::string, CCObject*> *dict = CCFileUtils::dictionaryWithContentsOfFileThreadSafe(plistFile.c_str());
    if ( !dict )
    {
        CCLOGERROR("CCBigImage#prepareTilesWithFile:extension:z: can not load dictionary from file: %s", plistFile.c_str());
        return;
    }
	
	// load image size
    CCDictionary<std::string, CCObject*> *sourceDict = (CCDictionary<std::string, CCObject*>*)dict->objectForKey(std::string("Source"));
	this->setContentSize(CCSizeFromString(valueForKey("Size", sourceDict)));
	
	// load tiles
    CCMutableArray<CCObject*>* array = (CCMutableArray<CCObject*>*)dict->objectForKey(std::string("Tiles"));
	
	_dynamicChildren = CCArray::arrayWithCapacity(array->count());
    _dynamicChildren->retain();
    
	// set screenLoadRectExtension = size of first tile
	if (array->count())
	{
        CCDictionary<std::string, CCObject*> *dict_ = (CCDictionary<std::string, CCObject*>*)array->getObjectAtIndex(0);
        _screenLoadRectExtension = CCRectFromString(valueForKey("Rect", dict_)).size;
	}
	
	//read data and create nodes and add them
    for (int i=0; i<array->count(); i++)
    {
        CCDictionary<std::string, CCObject*>* tileDict = (CCDictionary<std::string, CCObject*>*)array->getObjectAtIndex(i);

        // All properties of Dictionary
        const char *spriteName = valueForKey("Name", tileDict);
		
		CCRect tileRect = CCRectFromString(valueForKey("Rect", tileDict));

		
		// convert rect origin from top-left to bottom-left
		tileRect.origin.y = this->getContentSize().height - tileRect.origin.y - tileRect.size.height;
		
		// Use forced tile extension or original if tilesExtension isn't set
		if (!extension.empty())
		{
			// Change extension
            string filename = string(spriteName);
            int index = filename.find('.');
            string name = filename.substr(0, index);
			spriteName = (name+"."+extension).c_str();
		}
		
		// if file doesn't exist - do not use it
        /*int len = 10;
        string path = CCFileUtils::fullPathFromRelativePath(spriteName);
        CCLog("path=%s", path.c_str());
        if (!CCFileUtils::getFileData(path.c_str(), "r", (unsigned long *)(&len))) {
            CCLOGINFO(@"CCBigImage#prepareTilesWithFile:extension:z: %@ doesn't exists - skipping tile.", filePath);
			continue;
        }*/
        
		// Create & Add Tile (Dynamic Sprite Mode)
		UnloadableSpriteNode* tile = UnloadableSpriteNode::nodeWithImageForRect(spriteName, tileRect);
		this->addChild(tile, tilesZ);
		_dynamicChildren->addObject(tile);
		
    } //< for dict in arr
    
    dict->release();
	
}

CCBigImage::~CCBigImage()
{
    CCDirector::sharedDirector()->setDepthTest(true);

	_dynamicChildren->release();
	_dynamicChildren = NULL;
		
    CCSpriteFrameCache::sharedSpriteFrameCache()->removeUnusedSpriteFrames();
    _textureCache->removeUnusedTextures();
}


#pragma mark CCNode LifeCycle

void CCBigImage::onEnter()
{
    CCNode::onEnter();
	this->startTilesLoadingThread();
}

void CCBigImage::onExit()
{
	// turn off dynamic thread
	this->stopTilesLoadingThread();
    
    CCNode::onExit();
}

void CCBigImage::visit()
{
	this->updateLoadRect();
	
	CCNode::visit();
	
	// remove unused textures periodically
    static int i = CCBIGIMAGE_TEXTURE_UNLOAD_PERIOD;
    if (--i <= 0) {
        i = CCBIGIMAGE_TEXTURE_UNLOAD_PERIOD;
        if (_tilesLoadThreadIsSleeping) {
            _textureCache->removeUnusedTextures();
        }
    }
}

#if CC_BIGIMAGE_DEBUG_DRAW
void CCBigImage::draw()
{
    CCNode::draw();
	
	CCSize s = this->getContentSize();
	CCPoint vertices[4]={
		ccp(0,0),ccp(s.width,0),
		ccp(s.width,s.height),ccp(0,s.height),
	};
	ccDrawPoly(vertices, 4, true);
}
#endif


#pragma mark Dynamic Tiles Stuff

void CCBigImage::startTilesLoadingThread()
{
	// do nothing if thread exist
	if (this->_tilesLoadThread) {
		return;
	}
    
	// create new thread if it doesn't exist
    _tilesLoadThreadCancel = false;
    pthread_create(&_tilesLoadThread, NULL, &CCBigImage::updateTilesInThread, this);

	_tilesLoadThreadIsSleeping = false;
}

void CCBigImage::stopTilesLoadingThread()
{
    _tilesLoadThreadCancel = true;
	_tilesLoadThread = NULL;
}

void CCBigImage::updateLoadRect()
{
	// get screen rect
	CCRect screenRect = CCRectZero;
	screenRect.size = CCDirector::sharedDirector()->getWinSize();
	
	screenRect.size.width *= CC_CONTENT_SCALE_FACTOR();
	screenRect.size.height *= CC_CONTENT_SCALE_FACTOR();

	screenRect = CCRectApplyAffineTransform(screenRect, this->worldToNodeTransform());

	screenRect.origin = ccpMult(screenRect.origin, 1/CC_CONTENT_SCALE_FACTOR() );
	screenRect.size.width /= CC_CONTENT_SCALE_FACTOR();
	screenRect.size.height /= CC_CONTENT_SCALE_FACTOR();
    
	// get level's must-be-loaded-part rect
	_loadedRect = CCRectMake(screenRect.origin.x - _screenLoadRectExtension.width,
							 screenRect.origin.y - _screenLoadRectExtension.height,
							 screenRect.size.width + 2.0f * _screenLoadRectExtension.width,
							 screenRect.size.height + 2.0f * _screenLoadRectExtension.height);
	
	// avoid tiles blinking
	if (_significantPositionChange)
	{
		this->updateTiles();
		_significantPositionChange = false;
	}
}


// new update tiles for threaded use
void *CCBigImage::updateTilesInThread(void *ptr)
{
    CCThread thread;
    thread.createAutoreleasePool();
    
    CCBigImage* bigImage = (CCBigImage*)ptr;
    
	while(!bigImage->_tilesLoadThreadCancel)
	{
        CCThread thread;
        thread.createAutoreleasePool();
        
		// flag for removeUnusedTextures only when sleeping - to disable deadLocks
		bigImage->_tilesLoadThreadIsSleeping = false;
		
		for (int i=0; i<bigImage->_dynamicChildren->count(); i++)
        {
            UnloadableSpriteNode *child = (UnloadableSpriteNode*)(bigImage->_dynamicChildren->objectAtIndex(i));
			if (!CCRect::CCRectIntersectsRect(child->boundingBox(), bigImage->_loadedRect)) {
				child->unload();
            } else {
				child->load(bigImage->_textureCache);;
            } //< 0 == size.width must be faster than CGRectIsEmpty
		}
        
		// flag removeUnusedTextures only when sleeping - to disable deadLocks
		bigImage->_tilesLoadThreadIsSleeping = true;
		
		// 60 FPS run, update at 30 fps should be ok
        usleep(30*1000);
	}
    
    return NULL;
}

void CCBigImage::updateTiles()
{
	//load loadedRect tiles and unload tiles that are not in loadedRect
    for (int i=0; i<_dynamicChildren->count(); i++)
    {
        UnloadableSpriteNode *child = (UnloadableSpriteNode*)_dynamicChildren->objectAtIndex(i);
		if (!CCRect::CCRectIntersectsRect( _loadedRect, child->boundingBox())) {
			child->unload();
		} else {
			child->load(_textureCache);
        }
        //< 0 == size.width must be faster than CGRectIsEmpty
    }
}
