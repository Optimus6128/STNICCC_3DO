#define ENABLE_SOUND	1
#include "types.h"
#include "mem.h"
#include "audio.h"
#include "debug3do.h"
#include "effectshandler.h"
#include "event.h"
#include "nodes.h"
#include "kernelnodes.h"

/* added for spoolsound */
#include "debug.h"
#include "operror.h"
#include "filefunctions.h"
#include "music.h"

#define	DBUG(x)	/* PRT(x) */

#define     MAXVOICES   8
#define     NUMVOICES   8
#define     NUMCHANNELS 8
#define     NUMSAMPLERS 8
#define     MAXAMPLITUDE MAXDSPAMPLITUDE
#define     MAX_PITCH   60
#define     PITCH_RANGE 40

int32 PlaySoundFile (char *FileName, int32 BufSize, int32 NumReps);
void SpoolSoundFileThread( void );

/*
** Allocate enough space so that you don't get stack overflows.
** An overflow will be characterized by seemingly random crashes
** that defy all attempts at logical analysis.  You might want to
** start big then reduce the size till you crash, then double it.
*/
#define STACKSIZE (10000)

#define CHECKPTR(val,name) \
	if (val == 0) \
	{ \
		Result = -1; \
		ERR(("Failure in %s\n", name)); \
		goto error; \
	}

#define	DBUG(x)	/* PRT(x) */

#define NUMBLOCKS (64)
#define BLOCKSIZE (2048)
#define BUFSIZE (NUMBLOCKS*BLOCKSIZE)
#define NUMBUFFS  (2)
//#define MAXAMPLITUDE (0x7FFF)



/********* Globals for Thread **********/
static char *gFileName;
static int32 gSignal1;
static Item gMainTaskItem;
static int32 gNumReps;
static Item SpoolerThread;




/**************************************************************************
** Entry point for background thread.
**************************************************************************/
void SpoolSoundFileThread( void )
{
	int32 Result;

	// Initialize audio, return if error.
	if (OpenAudioFolio())
	{
		ERR(("Audio Folio could not be opened!\n"));
	}

	Result = PlaySoundFile ( gFileName, BUFSIZE, gNumReps);
	SendSignal( gMainTaskItem, gSignal1 );

	CloseAudioFolio();
	WaitSignal(0);   // Waits forever. Don't return!

}

int32 PlaySoundFile (char *FileName, int32 BufSize, int32 NumReps)
{
	int32 Result=0;
	SoundFilePlayer *sfp;
	int32 SignalIn, SignalsNeeded;
	int32 LoopCount;


	for( LoopCount = 0; LoopCount < NumReps; LoopCount++)
	{
		PRT(("Loop #%d\n", LoopCount));

		sfp = OpenSoundFile(FileName, NUMBUFFS, BufSize);
		CHECKPTR(sfp, "OpenSoundFile");

		Result = StartSoundFile( sfp, MAXAMPLITUDE );
		CHECKRESULT(Result,"StartSoundFile");

/* Keep playing until no more samples. */
		SignalIn = 0;
		SignalsNeeded = 0;
		do
		{
			if (SignalsNeeded) SignalIn = WaitSignal(SignalsNeeded);
			Result = ServiceSoundFile(sfp, SignalIn, &SignalsNeeded);
			CHECKRESULT(Result,"ServiceSoundFile");
		} while (SignalsNeeded);

		Result = StopSoundFile (sfp);
		CHECKRESULT(Result,"StopSoundFile");

	Result = CloseSoundFile (sfp);
	CHECKRESULT(Result,"CloseSoundFile");

	}

	return 0;

error:
	return (Result);
}

void startMusic()
{
  	int32 Priority;
    gFileName = "Music/music_16_stereo.aiff";


	gNumReps = 256;

    // Get parent task Item so that thread can signal back.
	gMainTaskItem = KernelBase->kb_CurrentTask->t.n_Item;

    // Allocate a signal for each thread to notify parent task.
	gSignal1 = AllocSignal(0);
	CHECKRESULT(gSignal1,"AllocSignal");

	Priority = 180;
	SpoolerThread = CreateThread("SoundSpooler", Priority, SpoolSoundFileThread, STACKSIZE);
	CHECKRESULT(SpoolerThread,"CreateThread");
}

void endMusic()
{
	DeleteThread( SpoolerThread );
	CloseAudioFolio();
}
