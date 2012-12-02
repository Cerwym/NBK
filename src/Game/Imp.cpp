#include "commons.h"
#include "Imp.h"
#include <math.h>

using namespace game_utils;

namespace game_objects
{
	CImp::CImp(): CCreature()
	{
		m_impState = IS_IDLE;
		m_MoveSpeed = 0.0005f;
		m_maxgold = 250;
	}

	CImp::~CImp()
	{
	}

	GLvoid CImp::checkGoldLevels()
	{
		if(this->getGold() >= this->m_maxgold)
		{
			depositGold();
		}
	}

	GLvoid CImp::checkNearestForDigging()
	{
		if(m_impState != IS_IDLE) return;

		std::vector<CBlock*> markedBlocks,possBlocks;
		std::vector<CBlock*>::iterator markedBlocksIter;
		CBlock *block;

		if(CV_GAME_MANAGER->getLevelManager()->isBlockTypeNear(CV_BLOCK_TYPE_EARTH_ID, cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),
			(int)floor(mPosition[2]/CV_BLOCK_DEPTH)),true,CV_PLAYER_UNDEFINED,&markedBlocks))
		{
			int oldSearchLimit = CV_GAME_MANAGER->getPathManager()->getSearchLimit();
			CV_GAME_MANAGER->getPathManager()->setSearchLimit(2);
			bool oldDiagonalMoves = CV_GAME_MANAGER->getPathManager()->getDiagonalMoves();
			CV_GAME_MANAGER->getPathManager()->setDiagonalMoves(false);
			
			for(markedBlocksIter=markedBlocks.begin(); markedBlocksIter!=markedBlocks.end(); markedBlocksIter++)
			{
				block = *markedBlocksIter;
				if(!block->isMarked())
					continue;
				path.clear();
				if(CV_GAME_MANAGER->getPathManager()->findPath(cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH)),block->getLogicalPosition(),&path))
					possBlocks.push_back(block);
			}

			CV_GAME_MANAGER->getPathManager()->setSearchLimit(oldSearchLimit);
			CV_GAME_MANAGER->getPathManager()->setDiagonalMoves(oldDiagonalMoves);
			if(possBlocks.size()>0)
			{
				GLint blockNum = rand()%possBlocks.size();
				mCurrentBlock = possBlocks[blockNum];
				path.clear();
				m_impState = IS_AT_DIGGING_BLOCK;
				return;
			}
		}
	}

	GLvoid CImp::checkNearestForClaiming()
	{
		if(m_impState != IS_IDLE) return;

		std::vector<CBlock*> unclaimedBlocks,claimedBlocks,possBlocks;
		std::vector<CBlock*>::iterator unclaimedBlocksIter, claimedBlocksIter;
		CBlock *block;

		if(CV_GAME_MANAGER->getLevelManager()->isBlockTypeNear(CV_BLOCK_TYPE_UNCLAIMED_LAND_ID,cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH)),true,CV_PLAYER_UNDEFINED,&unclaimedBlocks))
		{
			int oldSearchLimit = CV_GAME_MANAGER->getPathManager()->getSearchLimit();
			CV_GAME_MANAGER->getPathManager()->setSearchLimit(2);
			for(unclaimedBlocksIter=unclaimedBlocks.begin(); unclaimedBlocksIter!=unclaimedBlocks.end(); unclaimedBlocksIter++)
			{
				block = *unclaimedBlocksIter;
				if(block->isTaken())
					continue;
				claimedBlocks.clear();
				if(CV_GAME_MANAGER->getLevelManager()->isBlockClaimable(block->getLogicalPosition(),this->getOwner(),&claimedBlocks))
				{
					path.clear();
					if(CV_GAME_MANAGER->getPathManager()->findPath(cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH)),block->getLogicalPosition(),&path))
						possBlocks.push_back(block);
				}
			}
			CV_GAME_MANAGER->getPathManager()->setSearchLimit(oldSearchLimit);
			if(possBlocks.size()>0)
			{
				GLint blockNum = rand()%possBlocks.size();
				path.clear();
				mCurrentBlock = possBlocks[blockNum];
				path.push_back(mCurrentBlock->getLogicalPosition());
				mCurrentBlock->setTaken(true);
				m_impState = IS_GOING_TO_CLAIMING_DESTINATION;
				return;
			}
		}
	}

	GLvoid CImp::checkNearestForWalling()
	{
		if(m_impState != IS_IDLE) return;

		std::vector<CBlock*> unfortifiedBlocks,claimedBlocks,possBlocks;
		std::vector<CBlock*>::iterator unfortifiedBlocksIter;
		CBlock *block;

		if(CV_GAME_MANAGER->getLevelManager()->isBlockTypeNear(CV_BLOCK_TYPE_EARTH_ID,cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH)),true,CV_PLAYER_UNDEFINED,&unfortifiedBlocks))
		{
			int oldSearchLimit = CV_GAME_MANAGER->getPathManager()->getSearchLimit();
			CV_GAME_MANAGER->getPathManager()->setSearchLimit(2);
			bool oldDiagonalMoves = CV_GAME_MANAGER->getPathManager()->getDiagonalMoves();
			CV_GAME_MANAGER->getPathManager()->setDiagonalMoves(false);
			for(unfortifiedBlocksIter=unfortifiedBlocks.begin(); unfortifiedBlocksIter!=unfortifiedBlocks.end(); unfortifiedBlocksIter++)
			{
				block = *unfortifiedBlocksIter;
				claimedBlocks.clear();
				if(CV_GAME_MANAGER->getLevelManager()->isBlockClaimable(block->getLogicalPosition(),this->getOwner(),&claimedBlocks))
				{
					path.clear();
					if(CV_GAME_MANAGER->getPathManager()->findPath(cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH)),block->getLogicalPosition(),&path))
						possBlocks.push_back(block);
				}
			}
			CV_GAME_MANAGER->getPathManager()->setSearchLimit(oldSearchLimit);
			CV_GAME_MANAGER->getPathManager()->setDiagonalMoves(oldDiagonalMoves);
			if(possBlocks.size()>0)
			{
				GLint blockNum = rand()%possBlocks.size();
				mCurrentBlock = possBlocks[blockNum];
				path.clear();
				m_impState = IS_AT_WALLING_BLOCK;
				return;
			}
		}
	}

	GLvoid CImp::checkForDigging()
	{
		if(m_impState != IS_IDLE) return;
		path.clear();
		mCurrentBlock = CV_GAME_MANAGER->getLevelManager()->getMarkedBlock(this->getOwner(),cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH)));
		//bool oldEndOnDiagonal = CV_GAME_MANAGER->getPathManager()->getAllowEndDiagonal();
		CV_GAME_MANAGER->getPathManager()->setAllowEndDiagonal(false);
		if (mCurrentBlock)
		{
			cml::vector2i currPos = cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH));
			if(CV_GAME_MANAGER->getPathManager()->findPath(currPos,mCurrentBlock->getLogicalPosition(),&path) && !mCurrentBlock->isLow())
				m_impState = IS_GOING_TO_DIGGING_DESTINATION;
		}
		CV_GAME_MANAGER->getPathManager()->setAllowEndDiagonal(true);
	}

	GLvoid CImp::checkForClaiming()
	{
		if(m_impState != IS_IDLE) return;
		path.clear();
		mCurrentBlock = CV_GAME_MANAGER->getLevelManager()->getUnclaimedBlock(this->getOwner(),cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH)));
		if (mCurrentBlock)
		{
			cml::vector2i currPos = cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH));
			if(CV_GAME_MANAGER->getPathManager()->findPath(currPos,mCurrentBlock->getLogicalPosition(),&path))
				m_impState = IS_GOING_TO_CLAIMING_DESTINATION;
			else
				mCurrentBlock->setTaken(false);
		}
	}

	GLvoid CImp::checkForWalling()
	{
		if(m_impState != IS_IDLE) return;
		path.clear();
		mCurrentBlock = CV_GAME_MANAGER->getLevelManager()->getUnfortifiedBlock(this->getOwner(),cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH)));
		//bool oldEndOnDiagonal = CV_GAME_MANAGER->getPathManager()->getAllowEndDiagonal();
		CV_GAME_MANAGER->getPathManager()->setAllowEndDiagonal(false);
		if (mCurrentBlock)
		{
			cml::vector2i currPos = cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH));
			if(CV_GAME_MANAGER->getPathManager()->findPath(currPos,mCurrentBlock->getLogicalPosition(),&path) && !mCurrentBlock->isLow())
				m_impState = IS_GOING_TO_WALLING_DESTINATION;
		}
		CV_GAME_MANAGER->getPathManager()->setAllowEndDiagonal(true);
	}

	GLvoid CImp::faceBlock(CBlock *block)
	{
		mRotation[1] = 90.0f-(float)(atan2(block->getLogicalPosition()[1]-floor(mPosition[2]/CV_BLOCK_WIDTH),block->getLogicalPosition()[0]-floor(mPosition[0]/CV_BLOCK_WIDTH))*180.0f/M_PI);
	}

	GLvoid CImp::walkPath(GLfloat deltaTime)
	{
		//TODO: if imps are high enough level - teleport them
		if(path.size() == 0)
		{
			if(m_impState == IS_GOING_TO_DEPOSITING_GOLD_DESTINATION)
				m_impState = IS_AT_DEPOSITING_GOLD;
			else if(m_impState == IS_GOING_TO_CLAIMING_DESTINATION)
				m_impState = IS_AT_CLAIMING_BLOCK;
			else if (m_impState == IS_GOING_TO_DIGGING_DESTINATION)
				m_impState = IS_AT_DIGGING_BLOCK;
			else if (m_impState == IS_GOING_TO_WALLING_DESTINATION)
				m_impState = IS_AT_WALLING_BLOCK;
			return;
		}
		cml::vector2i point = path.back();
		GLfloat tX = (GLfloat)point[0]*CV_BLOCK_WIDTH+CV_BLOCK_WIDTH/2.0f;
		GLfloat tZ = (GLfloat)point[1]*CV_BLOCK_DEPTH+CV_BLOCK_DEPTH/2.0f;

		if (fabs(mPosition[0]-tX)<=0.01f && fabs(mPosition[2]-tZ)<=0.01f)
		{
			path.pop_back();
			if(path.size()==0)
			{
				mMoveVector[0] = 0.0f;
				mMoveVector[2] = 0.0f;
				return;
			}
			point = path.back();
			tX = (GLfloat)point[0]*CV_BLOCK_WIDTH+CV_BLOCK_WIDTH/2.0f;
			tZ = (GLfloat)point[1]*CV_BLOCK_DEPTH+CV_BLOCK_DEPTH/2.0f;
		}
		
		//calculate new movement direction
		mMoveVector[0] = tX-mPosition[0];
		mMoveVector[2] = tZ-mPosition[2];
		mMoveVector.normalize();
		
		mRotation[1] = 90.0f-(float)(atan2(mMoveVector[2],mMoveVector[0])*180.0f/M_PI);

		cml::vector3f oldPos = mPosition;
		mPosition += mMoveVector*m_MoveSpeed*deltaTime;

		if ((mPosition[0]-tX > 0.0f && oldPos[0]-tX<0.0f)
			|| (mPosition[0]-tX < 0.0f && oldPos[0]-tX>0.0f))
			mPosition[0] = tX;
		if ((mPosition[2]-tZ > 0.0f && oldPos[2]-tZ<0.0f)
			|| (mPosition[2]-tZ < 0.0f && oldPos[2]-tZ>0.0f))
			mPosition[2] = tZ;
			
	}

	GLvoid CImp::update(GLfloat deltaTime)
	{
		if (m_impState == IS_IDLE)
		{
			Idle(deltaTime);
			//check for next space digging
			//check for next space claiming
			//check for next space walling

			//check for digging
			//check for claiming
			//check for walling

			checkNearestForDigging();
			checkNearestForClaiming();
			checkNearestForWalling();

			checkForDigging();
			checkForClaiming();
			checkForWalling();
		} else if (m_impState == IS_GOING_TO_DEPOSITING_GOLD_DESTINATION)
		{
			useAction(AA_WALK);
			walkPath(deltaTime);		
		} else if (m_impState == IS_GOING_TO_DIGGING_DESTINATION)
		{
			useAction(AA_WALK);
			walkPath(deltaTime);
		} else if (m_impState == IS_GOING_TO_CLAIMING_DESTINATION)
		{
			useAction(AA_WALK);
			walkPath(deltaTime);
		} else if (m_impState == IS_GOING_TO_WALLING_DESTINATION)
		{
			useAction(AA_WALK);
			walkPath(deltaTime);
		} else if (m_impState == IS_AT_DEPOSITING_GOLD)
		{
			m_impState = IS_DEPOSITING_GOLD;
		} else if (m_impState == IS_AT_DIGGING_BLOCK)
		{
			faceBlock(mCurrentBlock);
			m_impState = IS_DIGGING;
			useAction(AA_DIG);
		} else if (m_impState == IS_AT_CLAIMING_BLOCK)
		{
			m_impState = IS_CLAIMING;
			useAction(AA_CLAIM);
		} else if (m_impState == IS_AT_WALLING_BLOCK)
		{
			faceBlock(mCurrentBlock);
			m_impState = IS_WALLING;
			useAction(AA_CLAIM);
		} else if (m_impState == IS_DEPOSITING_GOLD)
		{
			//Todo: if a block has a 250 gold, upp it to 500...
			bool found = false;
			for (std::vector<block_objects::CBlockObject*>::iterator rmIter = mCurrentBlock->getBlockObjects()->begin(); rmIter != mCurrentBlock->getBlockObjects()->end(); rmIter++)
			{
				block_objects::CBlockObject *bObject = *rmIter;

				if (bObject->getName() == "MODEL_GOLD250")
					found = true;

			}
			if(!found)
			{
				mCurrentBlock->addModel("MODEL_GOLD250",mPosition);
				PLAYER0_MONEY = PLAYER0_MONEY + this->getGold();//do for other teams
				this->setGold(0);	
				m_impState = IS_IDLE;
				useAction(AA_IDLE);
			}
			else
			{
				checkGoldLevels();
			}
		} else if (m_impState == IS_DIGGING)
		{
			if(mCurrentBlock->isLow() || !mCurrentBlock->isMarked())
			{
				m_impState = IS_IDLE;
				useAction(AA_IDLE);
				return;
			}
			if(mCurrentBlock->getType() == CV_BLOCK_TYPE_GOLD_ID)
				//TODO: use a new formula
				this->setGold(this->getGold() + 1);
			mCurrentBlock->decLife(deltaTime*m_MoveSpeed);
			if (mCurrentBlock->getLife()<=0.0f)
			{
				mCurrentBlock->digBlock();

				m_impState = IS_IDLE;
				useAction(AA_IDLE);
			}
			//Check if you have gold :)
			checkGoldLevels();
		} else if (m_impState == IS_CLAIMING)
		{
			mCurrentBlock->decLife(deltaTime*m_MoveSpeed);
			if (mCurrentBlock->getLife()<=0.0f)
			{
				mCurrentBlock->claimBlock(this->getOwner());
				m_impState = IS_IDLE;
				useAction(AA_IDLE);
			}
		} else if (m_impState == IS_WALLING)
		{
			if(mCurrentBlock->isLow() || mCurrentBlock->isMarked())
			{
				m_impState = IS_IDLE;
				useAction(AA_IDLE);
				return;
			}
			mCurrentBlock->addLife(deltaTime*m_MoveSpeed);
			if (mCurrentBlock->getLife()>=9.0f)
			{
				mCurrentBlock->fortifyBlock(this->getOwner());

				m_impState = IS_IDLE;
				useAction(AA_IDLE);
			}
		}
	}

	void CImp::depositGold()
	{
		path.clear();
		CBlock *destBlock;
		destBlock = CV_GAME_MANAGER->getRoomManager()->getRoom(CV_BLOCK_TYPE_TREASURE_ROOM_ID, this->getOwner());

		if(destBlock != NULL)
		{
			rooms::CRoom *currRoom = CV_GAME_MANAGER->getRoomManager()->getRoom(destBlock->getRoomIndex());

			for (std::vector<CBlock*>::iterator rmIter = currRoom->getRoomTilesVector()->begin(); rmIter != currRoom->getRoomTilesVector()->end(); rmIter++)
			{
				CBlock *thisBlock = *rmIter;
				bool found = false;

				block_objects::CBlockObject *bObject = thisBlock->GetBlockByName( "MODEL_GOLD250" );
				if (bObject != NULL)
				{
					found = true;
					mCurrentBlock = (game_objects::CBlock *)bObject;
					break;
				}
			}

			if(mCurrentBlock)
			{
				cml::vector2i currPos = cml::vector2i((int)floor(mPosition[0]/CV_BLOCK_WIDTH),(int)floor(mPosition[2]/CV_BLOCK_DEPTH));
				if(CV_GAME_MANAGER->getPathManager()->findPath(currPos,mCurrentBlock->getLogicalPosition(),&path))
				{
					m_impState = IS_GOING_TO_DEPOSITING_GOLD_DESTINATION;
					return;
				}
			}
		}
		mCurrentBlock->addModel("MODEL_GOLD250",mPosition);
		this->setGold(0);
	}

};