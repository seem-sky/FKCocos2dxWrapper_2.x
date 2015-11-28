//*************************************************************************
//	创建日期:	2015-1-9
//	文件名称:	FKAudioSystem.h
//  创 建 人:   王宏张 FreeKnight	
//	版权所有:	MIT
//	说    明:	
//*************************************************************************

#pragma once

//-------------------------------------------------------------------------
#include "../FKEngineCommonHead.h"
//-------------------------------------------------------------------------
class FKAudioSystem : public IAudioSystem
{
public:
	FKAudioSystem();
	~FKAudioSystem();
public:
	// 释放清理
	virtual void			Release();
	// 播放背景音乐
	virtual void			PlayBackgroundMusic(const char* pszFilePath, bool bLoop);
	// 停止背景音乐
	// 参数：bReleaseData 停止音乐后，是否继续保存音乐缓冲内存，默认值为false
	virtual void			StopBackgroundMusic(bool bReleaseData);
	// 获取背景音乐音量
	// 返回值：	【0.0f, 1.0f】
	virtual float			GetBackgroundMusicVolume();
	// 设置背景音乐音量
	// 参数：	【0.0f, 1.0f】
	virtual void			SetBackgroundMusicVolume(float volume);
	// 获取音效音量
	// 返回值：	【0.0f, 1.0f】
	virtual float			GetEffectsVolume();
	// 设置音效音量
	// 参数：	【0.0f, 1.0f】
	virtual void			SetEffectsVolume(float volume);
	// 播放音效
	virtual unsigned int	PlayEffect(const char* pszFilePath, bool bLoop);
	// Resume指定音效
	// 参数：PlayEffect返回的EffectID
	virtual void			ResumeEffect(unsigned int nSoundId);
	// Resume全部音效
	virtual void			ResumeAllEffects();
	// 停止指定音效
	// 参数：PlayEffect返回的EffectID
	virtual void			StopEffect(unsigned int nSoundId);
	// 停止全部音效
	virtual void			StopAllEffects();
	// 系统静音
	// 参数：是否静音
	virtual void			MuteAllSounds( bool bMute );
private:
	float					m_fEffectVolume;			// 特效音量
	float					m_fBackgroundMusicVolume;	// 背景音乐音量
};
//-------------------------------------------------------------------------
extern FKAudioSystem gs_FKAudioSystem;
//-------------------------------------------------------------------------