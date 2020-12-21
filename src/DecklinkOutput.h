/**
 * @file
 * @brief Header file for DecklinkOutput class
 * @author Jonathan Thomas <jonathan@openshot.org>, Blackmagic Design
 *
 * @ref License
 */

/* LICENSE
 *
 * Copyright (c) 2009 Blackmagic Design
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *
 * Copyright (c) 2008-2019 OpenShot Studios, LLC
 * <http://www.openshotstudios.com/>. This file is part of
 * OpenShot Library (libopenshot), an open-source project dedicated to
 * delivering high quality video editing and animation solutions to the
 * world. For more information visit <http://www.openshot.org/>.
 *
 * OpenShot Library (libopenshot) is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * OpenShot Library (libopenshot) is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenShot Library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPENSHOT_DECKLINK_OUTPUT_H
#define OPENSHOT_DECKLINK_OUTPUT_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "DeckLinkAPI.h"
#include "CacheMemory.h"
#include "Frame.h"

enum OutputSignal {
	kOutputSignalPip		= 0,
	kOutputSignalDrop		= 1
};

/// Implementation of the Blackmagic Decklink API (used by the DecklinkWriter)
class DeckLinkOutputDelegate : public IDeckLinkVideoOutputCallback, public IDeckLinkAudioOutputCallback
{
protected:
	unsigned long					m_totalFramesScheduled;
	OutputSignal					m_outputSignal;
	void*							m_audioBuffer;
	unsigned long					m_audioBufferSampleLength;
	unsigned long					m_audioBufferOffset;
	unsigned long					m_audioChannelCount;
	BMDAudioSampleRate				m_audioSampleRate;
	unsigned long					m_audioSampleDepth;
	unsigned long					audioSamplesPerFrame;
	unsigned long					m_framesPerSecond;
	int								height;
	int								width;

	unsigned long 							frameCount;
	//map<int, IDeckLinkMutableVideoFrame* > 	temp_cache;
	std::map<int, uint8_t * > 	temp_cache;

	BMDTimeValue frameRateDuration, frameRateScale;

	// Queue of raw video frames
	//deque<IDeckLinkMutableVideoFrame*> final_frames;
	std::deque<uint8_t * > final_frames;
	std::deque<std::shared_ptr<openshot::Frame> > raw_video_frames;

	// Convert between YUV and RGB
	IDeckLinkOutput *deckLinkOutput;
	IDeckLinkDisplayMode *displayMode;

	// Current frame being displayed
	IDeckLinkMutableVideoFrame *m_currentFrame;

public:
	DeckLinkOutputDelegate(IDeckLinkDisplayMode *displayMode, IDeckLinkOutput* deckLinkOutput);
	~DeckLinkOutputDelegate();

	// *** DeckLink API implementation of IDeckLinkVideoOutputCallback IDeckLinkAudioOutputCallback *** //
	// IUnknown needs only a dummy implementation
	virtual HRESULT STDMETHODCALLTYPE	QueryInterface (REFIID iid, LPVOID *ppv)	{return E_NOINTERFACE;}
	virtual ULONG STDMETHODCALLTYPE		AddRef ()									{return 1;}
	virtual ULONG STDMETHODCALLTYPE		Release ()									{return 1;}

	virtual HRESULT STDMETHODCALLTYPE	ScheduledFrameCompleted (IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result);
	virtual HRESULT STDMETHODCALLTYPE	ScheduledPlaybackHasStopped ();

	virtual HRESULT STDMETHODCALLTYPE	RenderAudioSamples (bool preroll);

	/// Schedule the next frame
	void ScheduleNextFrame(bool prerolling);

	/// Custom method to write new frames
	void WriteFrame(std::shared_ptr<openshot::Frame> frame);

private:
	ULONG				m_refCount;
	pthread_mutex_t		m_mutex;
};


#endif
