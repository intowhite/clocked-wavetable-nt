// Clocked Wavetable v1.0.0 -- Expert Sleepers disting NT native C++ algorithm
// MIT License. Source prototype; hardware ARM build requires official distingNT_API.
#include <distingnt/api.h>
#include <distingnt/wav.h>
#include <stdint.h>
#include <new>

namespace {
static constexpr uint32_t kTableSamples = 262144; // 512 KiB DRAM
static constexpr float kHighV=1.0f, kLowV=0.2f;
static constexpr double kPi=3.14159265358979323846;
#include "sin_lut.inc"

// Factory parameter IDs intentionally track page/visual order.
// This is a pre-v1 BREAKING change: a new instance starts with Mode (index 0)
// rather than Clock In, which previously caused the NT to open Inputs first.
enum Parameter {
    // Main — first parameter must live on the first visible page.
    kMode, kVoltageRange, kBank, kQuality, kPlayhead,
    // Shape
    kRate, kMorph, kSkew, kSquish, kPhaseOffset, kAmplitude, kOffsetMv, kCharacter,
    // Clock
    kClockSource, kClockFormat, kBarsPerPulse, kInternalBpm, kStopMode, kSyncStatus,
    // Inputs
    kClockIn, kTriggerIn, kResetIn, kMorphIn, kWarpIn,
    // Outputs
    kOutput, kOutputB, kPhaseB, kOutputC, kPhaseC, kOutputD, kPhaseD, kOutputMode,
    kNumParameters
};
enum { kLfo=0, kEnvelope=1, kGated=2 };
enum { kExternal=0, kMidi=1, kInternal=2, kTap=3 };
enum { kPulse4=0, kPulse24=1, kPulseBeat=2, kPulseBar=3, kPulseNBar=4 };
enum { kFreeRun=0, kFreeze=1 };
// Kermit-inspired lossy rendering.
enum { kClean=0, k8Bit=1, k6Bit=2, k4Bit=3, k2Bit=4,
       kGrainy32=5, kChoppy16=6, kChunky8=7, kCrushed4=8 };
static int stepCount(int character) {
    switch(character) {
        case kGrainy32: return 32;
        case kChoppy16: return 16;
        case kChunky8: return 8;
        case kCrushed4: return 4;
        default: return 0;
    }
}
struct Ratio { uint8_t cycles, beats; };
static constexpr Ratio kRatios[] = {
    {1,64},{1,48},{1,32},{1,24},{1,16},{1,12},{1,8},{1,6},
    {1,4},{1,3},{1,2},{1,1},{2,1},{3,1},{4,1},{6,1},
    {8,1},{12,1},{16,1},{24,1},{32,1},{48,1},{64,1}
};
static char const* const kRatioLabels[] = {
    "/64","/48","/32","/24","/16","/12","/8","/6","/4","/3","/2","x1",
    "x2","x3","x4","x6","x8","x12","x16","x24","x32","x48","x64"
};
static char const* const kModes[]={"LFO","Envelope","Gated"};
static char const* const kSources[]={"External","MIDI clock","Internal","Tap CV"};
// Most common CV sequencer format first, also the factory default.
static char const* const kFormats[]={"4 PPQN (1/16)","24 PPQN","1 PPQN (1/4)","1 Bar","Every N Bars"};
static char const* const kOnOff[]={"Off","On"};
static char const* const kQualityLabels[]={"Adaptive","Full rate"};
static char const* const kCharacters[]={"Clean","8-bit","6-bit","4-bit","2-bit",
    "Grainy 32","Choppy 16","Chunky 8","Crushed 4"};
static char const* const kStops[]={"Free run","Freeze"};
static char const* const kRanges[]={"-5 to +5V","0 to 5V","0 to 10V"};
// Must exactly match enum Parameter above. Default Clock Format is 4 PPQN.
static const _NT_parameter kParameters[] = {
    // Main
    {"Mode",0,2,0,kNT_unitEnum,0,kModes},
    {"Voltage",0,2,0,kNT_unitEnum,0,kRanges},
    {"Wavetable",0,4095,0,kNT_unitHasStrings,0,nullptr},
    {"CV quality",0,1,0,kNT_unitEnum,0,kQualityLabels},
    {"Playhead",0,1,1,kNT_unitEnum,0,kOnOff},
    // Shape
    {"Rate",0,22,11,kNT_unitEnum,0,kRatioLabels},
    {"Morph",0,1000,0,kNT_unitPercent,kNT_scaling10,nullptr},
    {"Warp",5,95,50,kNT_unitPercent,0,nullptr},
    {"Squish",-100,100,0,kNT_unitPercent,0,nullptr},
    {"Phase",0,359,0,kNT_unitDegrees,0,nullptr},
    {"Amplitude",0,100,100,kNT_unitPercent,0,nullptr},
    {"Offset",-5000,5000,0,kNT_unitMillivolts,0,nullptr},
    {"Character",0,8,0,kNT_unitEnum,0,kCharacters},
    // Clock
    {"Clock source",0,3,0,kNT_unitEnum,0,kSources},
    {"Clock format",0,4,0,kNT_unitEnum,0,kFormats},
    {"N Bars",2,128,4,kNT_unitNone,0,nullptr},
    {"Internal BPM",20,300,120,kNT_unitBPM,0,nullptr},
    {"On clock stop",0,1,0,kNT_unitEnum,0,kStops},
    {"Sync status",0,0,0,kNT_unitHasStrings,0,nullptr},
    // Inputs
    {"Clock in",0,kNT_lastBus,1,kNT_unitCvInput,0,nullptr},
    {"Trigger/gate",0,kNT_lastBus,0,kNT_unitCvInput,0,nullptr},
    {"Reset in",0,kNT_lastBus,0,kNT_unitCvInput,0,nullptr},
    {"Morph CV",0,kNT_lastBus,0,kNT_unitCvInput,0,nullptr},
    {"Warp CV",0,kNT_lastBus,0,kNT_unitCvInput,0,nullptr},
    // Outputs
    {"Output A",1,kNT_lastBus,13,kNT_unitCvOutput,0,nullptr},
    {"Phased output B",0,kNT_lastBus,0,kNT_unitCvOutput,0,nullptr},
    {"Phase B",0,359,90,kNT_unitDegrees,0,nullptr},
    {"Phased output C",0,kNT_lastBus,0,kNT_unitCvOutput,0,nullptr},
    {"Phase C",0,359,180,kNT_unitDegrees,0,nullptr},
    {"Phased output D",0,kNT_lastBus,0,kNT_unitCvOutput,0,nullptr},
    {"Phase D",0,359,270,kNT_unitDegrees,0,nullptr},
    {"Output mode",0,1,1,kNT_unitOutputMode,0,nullptr},
};
static_assert(sizeof(kParameters)/sizeof(kParameters[0])==kNumParameters,"Parameter mismatch");
static_assert(sizeof(kRatioLabels)/sizeof(kRatioLabels[0])==sizeof(kRatios)/sizeof(kRatios[0]),"Rate labels");
// High-level setup lives on Main; all the hands-on modulation controls are
// on Shape, including Rate, Morph, Character and DC Offset.
static const uint8_t kMainPage[]={kMode,kVoltageRange,kBank,kQuality,kPlayhead};
static const uint8_t kShapePage[]={kRate,kMorph,kSkew,kSquish,kPhaseOffset,kAmplitude,kOffsetMv,kCharacter};
// Parameter IDs follow the page order, and membership is built per instance.
// Recalculate the host's visible pages whenever a controlling setting changes.
// This requires disting NT API v13 (NT_updateParameterPages).
static const uint8_t kInputsDefault[]={kClockIn,kResetIn,kMorphIn,kWarpIn};
static const uint8_t kOutputsDefault[]={kOutput,kOutputB,kOutputC,kOutputD,kOutputMode};
// A valid default page set is also available before the host assigns v[].
static const uint8_t kClockDefault[]={kClockSource,kClockFormat,kSyncStatus,kStopMode};
static const _NT_parameterPage kDefaultPages[]={
    {"Main",5,0,{0,0},kMainPage}, {"Shape",8,0,{0,0},kShapePage},
    {"Clock",4,0,{0,0},kClockDefault}, {"Inputs",4,0,{0,0},kInputsDefault},
    {"Outputs",5,0,{0,0},kOutputsDefault}
};
static const _NT_parameterPages kDefaultPageSet={5,kDefaultPages};
static float clampf(float x,float lo,float hi){return x<lo?lo:(x>hi?hi:x);}
static double clampd(double x,double lo,double hi){return x<lo?lo:(x>hi?hi:x);}
// The NT plug-in loader does not necessarily provide the libgcc helpers for
// conversion between doubles and 64-bit integers. Phase is bounded on normal
// running timescales, so 32-bit conversion is sufficient. This intentionally
// avoids __aeabi_{d2lz,d2ulz,l2d,ul2d} runtime imports.
static double fraction(double x){
    if(x<0.0){
        const int32_t whole=static_cast<int32_t>(x);
        x-=static_cast<double>(whole);
        if(x<0.0)x+=1.0;
        return x;
    }
    return x-static_cast<double>(static_cast<uint32_t>(x));
}
static double ratioAt(int i) {
    if(i<0||i>=static_cast<int>(sizeof(kRatios)/sizeof(kRatios[0])))i=11;
    return static_cast<double>(kRatios[i].cycles)/kRatios[i].beats;
}
// Smooth monotonic phase warp: u(0)=0, u(skew)=0.5, u(1)=1.
// Differentiable everywhere on (0,1), unlike the previous piecewise-linear map.
static float warpSkew(float t,float skew) {
    skew=clampf(skew,0.05f,0.95f);
    const float a=(1.0f-skew)/skew;
    return (a*t)/(1.0f+(a-1.0f)*t);
}
static float sineCycle(float t) {
    float pos=clampf(t,0.0f,1.0f)*256.0f;
    uint32_t i=static_cast<uint32_t>(pos);
    if(i>=256)return kSinCycle[256];
    return kSinCycle[i]+(kSinCycle[i+1]-kSinCycle[i])*(pos-i);
}
// Squish changes intracycle timing but keeps both endpoints AND midpoint fixed.
// derivative = 1 + s*cos(2*pi*t), with |s|<=0.90; strictly monotonic.
static float warpSquish(float t,float squish) {
    return t+(0.9f*squish)*sineCycle(t)*static_cast<float>(1.0/(2.0*kPi));
}
// Warp CV adds 9 percentage points per volt to the knob setting.
// +/-5 V spans +/-45 points, with the final value constrained to 5..95%.
// Zero volts never changes the existing saved Warp/Skew parameter.
struct ClockedWT;
static float effectiveWarp(const ClockedWT* wt,float warpCv); // defined after ClockedWT
static float wrap01(float phase){
    // Call only for phases in (-1,2); this fast wrap avoids float->int or
    // double->int runtime conversion in the per-sample DSP loop.
    if(phase>=1.0f)phase-=1.0f;
    if(phase<0.0f)phase+=1.0f;
    return phase;
}
static float warp(float phase,float skew,float squish) {
    return clampf(warpSkew(warpSquish(phase,squish),skew),0.0f,0.99999994f);
}
static bool rising(float volts,bool& latched) {
    if(latched) {if(volts<kLowV)latched=false; return false;}
    if(volts>kHighV){latched=true;return true;}
    return false;
}
// Separate interpolation/step-hold state per optional phase output.
// These caches are only used when the corresponding output bus is nonzero.
struct AuxRender {
    float waveA, waveB, held, lastMorph;
    int bucket, remaining, length;
    bool heldValid, renderValid;
    AuxRender():waveA(0),waveB(0),held(0),lastMorph(-1),bucket(-1),
        remaining(0),length(1),heldValid(false),renderValid(false){}
    void invalidate(){ heldValid=false;renderValid=false;remaining=0; }
};
struct ClockedWT: public _NT_algorithm {
    // Instance-owned dynamic pages: separate algorithm instances can expose
    // different Clock, Inputs and Outputs configurations simultaneously.
    _NT_parameterPage visiblePages[5];
    _NT_parameterPages visiblePageSet;
    uint8_t clockParams[6], inputParams[5], outputParams[8];
    _NT_parameter parametersOwned[kNumParameters];
    _NT_wavetableRequest request;
    int16_t* table;
    bool mounted,reading,ready;
    volatile bool readComplete;
    int activeBank;
    uint32_t wavetableCount;

    // 32-bit sample counters wrap naturally; subtraction remains correct for
    // the short clock/tap intervals we measure. No 64-bit float conversions.
    uint32_t samples,lastExternalEdge,lastTap;
    uint32_t edgeCount;
    // External 1-Bar and Every-N-Bars formats use bars as the timing unit; external PPQN,
    // internal BPM and Tap use quarter notes. Rate is per timing unit.
    double unitPosition;          // continuous units since clock epoch
    double clockOriginUnits;      // external phase reference
    double externalPulsePeriod;   // measured samples between rising edges
    double unitSamples;           // samples per bar OR per quarter note
    double pllTrimPerSample;      // slow continuous phase correction, no jumps on edges
    bool seenEdge,hasPeriod,clockHigh,triggerHigh,resetHigh,tapSeen,clockStopped;
    uint32_t midiTickCount, midiStartCount, midiStopCount;
    bool envActive;
    // MIDI realtime messages (F8 clock, FA start, FB continue, FC stop).
    // Source receives clock without Start too, as some clock senders omit
    // transport; an explicit Stop inhibits subsequent clock ticks until Start.
    bool midiRunning, midiTransportSeen;
    double envPhase;              // 0..1 one-shot or gated repeating
    float displayedPhase;
    float lastMorphCv;
    float lastWarpCv;
    // Adaptive control-rate rendering with linear interpolation. No 64-bit helpers.
    float waveA, waveB;
    // Stepped Character modes hold N positions per cycle, and update
    // immediately when Morph changes.
    float loFiHold;
    float loFiMorph;
    int loFiBucket;
    bool loFiValid;
    int renderRemain, renderLength;
    bool renderValid;
    uint32_t nextLibraryRetry;
    // Reuse the OLED trace until waveform-driving controls/CV change.
    int16_t previewYs[124];
    bool previewValid;
    float previewMorphCv,previewWarpCv;
    AuxRender aux[3];
    // Remember each optional routing selection so a first turn from Off can
    // suggest a free physical output without disrupting later manual choices.
    int16_t previousAuxBus[3];
    void invalidateAux(){for(int i=0;i<3;++i)aux[i].invalidate();}

    explicit ClockedWT(int16_t* storage):table(storage),mounted(false),reading(false),ready(false),
        readComplete(false),activeBank(-1),wavetableCount(0),samples(0),lastExternalEdge(0),lastTap(0),
        edgeCount(0),unitPosition(0),clockOriginUnits(0),externalPulsePeriod(0),unitSamples(24000),pllTrimPerSample(0),
        seenEdge(false),hasPeriod(false),clockHigh(false),triggerHigh(false),resetHigh(false),
        tapSeen(false),clockStopped(false),midiTickCount(0),midiStartCount(0),midiStopCount(0),
        envActive(false),midiRunning(true),midiTransportSeen(false),envPhase(0),displayedPhase(0),lastMorphCv(0),lastWarpCv(0),
        waveA(0),waveB(0),loFiHold(0),loFiMorph(-1),loFiBucket(-1),loFiValid(false),
        renderRemain(0),renderLength(1),renderValid(false),nextLibraryRetry(0),
        previewValid(false),previewMorphCv(0),previewWarpCv(0) {
        for(int i=0;i<3;++i)previousAuxBus[i]=0;
        // Volatile byte operations prevent compiler-generated memcpy/memset,
        // which may not be exported by the NT's plug-in loader.
        volatile uint8_t* dst=reinterpret_cast<volatile uint8_t*>(parametersOwned);
        const volatile uint8_t* src=reinterpret_cast<const volatile uint8_t*>(kParameters);
        for(uint32_t i=0;i<sizeof(parametersOwned);++i)dst[i]=src[i];
        volatile uint8_t* requestBytes=reinterpret_cast<volatile uint8_t*>(&request);
        for(uint32_t i=0;i<sizeof(request);++i)requestBytes[i]=0;
        request.table=table;
        request.tableSize=kTableSamples;
        request.callbackData=this;
        parameters=parametersOwned;
        parameterPages=&kDefaultPageSet;
        v=nullptr;vIncludingCommon=nullptr;
    }
};
static float effectiveWarp(const ClockedWT* wt,float warpCv){
    return clampf(wt->v[kSkew]*0.01f + warpCv*0.09f,0.05f,0.95f);
}
static void readFinished(void* data){static_cast<ClockedWT*>(data)->readComplete=true;}
static void calculateRequirements(_NT_algorithmRequirements& req,const int32_t*) {
    req.numParameters=kNumParameters; req.sram=sizeof(ClockedWT);
    req.dram=kTableSamples*sizeof(int16_t);req.dtc=0;req.itc=0;
}
static _NT_algorithm* construct(const _NT_algorithmMemoryPtrs& ptrs,const _NT_algorithmRequirements&,const int32_t*){
    return new (ptrs.sram) ClockedWT(reinterpret_cast<int16_t*>(ptrs.dram));
}
// Event-driven bank loading. During normal playback the audio callback only
// checks a volatile completion flag and the selected bank ID; it does *not*
// query the SD card or enumerate the library again.
//
// The firmware indexes wavetables at boot; newly copied banks require a
// firmware/library rescan or reboot. If the SD card is not available yet, or
// the wavetable index is temporarily empty, retry only once per second.
static void refreshLibrary(ClockedWT* wt){
    // The asynchronous read callback does nothing but set this flag.
    // Finalize on the audio thread (rather than in the SD callback).
    if(wt->reading && wt->readComplete){
        wt->readComplete=false;
        wt->reading=false;
        wt->ready=!wt->request.error &&
                  wt->request.numWaves>0 && wt->request.waveLength>0;
        wt->renderValid=false;wt->loFiValid=false;wt->invalidateAux();wt->previewValid=false;
        wt->activeBank=static_cast<int>(wt->request.index);
    }

    // Discover once after construction, unless the SD card or index isn't
    // ready yet. No time-based SD checks once a nonempty library is found.
    if(!wt->mounted || wt->wavetableCount==0){
        if(static_cast<int32_t>(wt->samples-wt->nextLibraryRetry)<0)return;
        uint32_t oneSecond=static_cast<uint32_t>(NT_globals.sampleRate);
        if(oneSecond==0)oneSecond=48000;
        wt->nextLibraryRetry=wt->samples+oneSecond;
        if(!NT_isSdCardMounted()){
            wt->mounted=false;
            wt->ready=false;
            wt->activeBank=-1;
            return;
        }
        wt->mounted=true;
        uint32_t count=NT_getNumWavetables();
        if(count>32768)count=32768;
        if(count!=wt->wavetableCount){
            wt->wavetableCount=count;
            wt->parametersOwned[kBank].max=count?
                static_cast<int16_t>(count-1):0;
            int index=NT_algorithmIndex(wt);
            if(index>=0)
                NT_updateParameterDefinition(static_cast<uint32_t>(index),kBank);
        }
        if(count==0)return;
    }

    // A load may take multiple blocks. Do not query SD status or the library
    // while waiting; the completion callback is our notification.
    if(wt->reading)return;
    const int wanted=wt->v[kBank];
    if(wanted<0 || static_cast<uint32_t>(wanted)>=wt->wavetableCount){
        wt->ready=false;
        return;
    }
    if(wanted==wt->activeBank)return;

    // Selecting a new bank is the only reason to initiate a read. Keep the
    // previous failure behavior: a failed load is not retried every block;
    // choose a different bank (or re-add the algorithm) to try again.
    wt->ready=false;
    wt->renderValid=false;wt->loFiValid=false;wt->invalidateAux();wt->previewValid=false;
    wt->request.index=static_cast<uint32_t>(wanted);
    wt->request.callback=readFinished;
    wt->request.error=false;
    wt->request.numWaves=0;
    wt->request.waveLength=0;
    wt->readComplete=false;
    if(NT_readWavetable(wt->request))wt->reading=true;
    else wt->activeBank=wanted;
}
// Number of clock units represented by one incoming external pulse.
// 1 Bar is a full clock unit regardless of time signature or BPM.
static double unitsPerPulse(const ClockedWT* wt){
    switch(wt->v[kClockFormat]){
        case kPulseBar:return 1.0;
        case kPulseNBar:return clampd(static_cast<double>(wt->v[kBarsPerPulse]),2.0,128.0);
        case kPulse24:return 1.0/24.0;
        case kPulse4:return 0.25;
        default:return 1.0;
    }
}
static void resetClock(ClockedWT* wt){
    wt->seenEdge=false;wt->hasPeriod=false;wt->edgeCount=0;wt->externalPulsePeriod=0;
    wt->clockHigh=false;wt->pllTrimPerSample=0;wt->clockStopped=false;wt->renderValid=false;wt->loFiValid=false;
    wt->unitPosition=0;wt->clockOriginUnits=0;
    // Internal/Tap uses BPM in quarter notes; for External 1 Bar, the
    // incoming bar interval is unknown until the second valid pulse.
    wt->unitSamples=NT_globals.sampleRate*60.0/wt->v[kInternalBpm];
}
static void clockTick(ClockedWT* wt,uint32_t sampleRate,double pulseUnits=0.0){
    // CV clocks may express a bar or fraction of a quarter note. MIDI clock
    // is invariably 24 ticks per quarter; it ignores the CV Clock Format.
    const bool fromMidi=pulseUnits>0.0;
    if(!fromMidi)pulseUnits=unitsPerPulse(wt);
    const bool barMode=!fromMidi&&(wt->v[kClockFormat]==kPulseBar || wt->v[kClockFormat]==kPulseNBar);
    const bool restarting=wt->clockStopped;
    wt->clockStopped=false;
    if(restarting){wt->seenEdge=false;wt->hasPeriod=false;wt->edgeCount=0;}
    if(!wt->seenEdge){
        wt->seenEdge=true;wt->edgeCount=0;
        if(!fromMidi&&(!restarting||barMode))wt->unitPosition=0;
        wt->clockOriginUnits=wt->unitPosition;
        wt->lastExternalEdge=wt->samples;wt->pllTrimPerSample=0;
        return;
    }
    uint32_t elapsed=wt->samples-wt->lastExternalEdge;
    wt->lastExternalEdge=wt->samples;
    // Long clocks can arrive only once every 128 bars: at 120 BPM in 4/4
    // that is 256 seconds, so the former 60-second cutoff was too short.
    // Retain a generous 1-hour upper bound to reject obviously invalid edges.
    if(elapsed<sampleRate/1000||static_cast<double>(elapsed)>sampleRate*3600.0)return;
    const bool firstMeasuredPulse=!wt->hasPeriod;
    double interval=static_cast<double>(elapsed);
    wt->externalPulsePeriod=wt->hasPeriod?0.75*wt->externalPulsePeriod+0.25*interval:interval;
    wt->hasPeriod=true;
    wt->unitSamples=wt->externalPulsePeriod/pulseUnits;
    ++wt->edgeCount;
    if(fromMidi&&firstMeasuredPulse){
        // The second MIDI/Tap pulse establishes tempo AND our first known
        // beat boundary. Align directly to that boundary instead of spending
        // several beats correcting a phase error through the gentle PLL.
        wt->unitPosition=wt->clockOriginUnits+pulseUnits;
        wt->pllTrimPerSample=0;
        wt->renderValid=false;wt->invalidateAux();
        return;
    }
    if(barMode&&firstMeasuredPulse){
        // We did not guess at the bar duration during the first incoming bar.
        // Align at this second edge, then advance using the measured duration.
        // Start at zero on the SECOND pulse, our first tempo-aware boundary.
        // Using +N here would skip ahead within divided cycles (/3, /4 etc).
        wt->unitPosition=0.0;
        wt->clockOriginUnits=-pulseUnits;
        wt->pllTrimPerSample=0;
        wt->renderValid=false;
        return;
    }
    double targetUnit=wt->clockOriginUnits+static_cast<double>(wt->edgeCount)*pulseUnits;
    double error=targetUnit-wt->unitPosition;
    double natural=1.0/wt->unitSamples;
    // Apply any correction *over time*, rather than jumping the current phase.
    wt->pllTrimPerSample=clampd(error/(2.0*wt->externalPulsePeriod),-0.15*natural,0.15*natural);
}
// Build a fresh page description for this particular plug-in instance.
// Page descriptors are built per instance for conditional menu visibility.
static void configurePages(ClockedWT* wt,bool notifyHost){
    uint8_t n=0;
    const int source=wt->v[kClockSource];
    const int format=wt->v[kClockFormat];
    wt->clockParams[n++]=kClockSource;
    if(source==kInternal)wt->clockParams[n++]=kInternalBpm;
    if(source==kExternal){
        wt->clockParams[n++]=kClockFormat;
        if(format==kPulseNBar)wt->clockParams[n++]=kBarsPerPulse;
    }
    // The status is deliberately adjacent to the source-dependent settings.
    wt->clockParams[n++]=kSyncStatus;
    if(source==kExternal)wt->clockParams[n++]=kStopMode;
    const uint8_t clockCount=n;

    n=0;
    if(source==kExternal||source==kTap)wt->inputParams[n++]=kClockIn;
    if(wt->v[kMode]!=kLfo)wt->inputParams[n++]=kTriggerIn;
    wt->inputParams[n++]=kResetIn;
    wt->inputParams[n++]=kMorphIn;
    wt->inputParams[n++]=kWarpIn;
    const uint8_t inputCount=n;

    n=0;
    wt->outputParams[n++]=kOutput;
    wt->outputParams[n++]=kOutputB;
    if(wt->v[kOutputB]!=0)wt->outputParams[n++]=kPhaseB;
    wt->outputParams[n++]=kOutputC;
    if(wt->v[kOutputC]!=0)wt->outputParams[n++]=kPhaseC;
    wt->outputParams[n++]=kOutputD;
    if(wt->v[kOutputD]!=0)wt->outputParams[n++]=kPhaseD;
    wt->outputParams[n++]=kOutputMode;
    const uint8_t outputCount=n;

    wt->visiblePages[0]={"Main",5,0,{0,0},kMainPage};
    wt->visiblePages[1]={"Shape",8,0,{0,0},kShapePage};
    wt->visiblePages[2]={"Clock",clockCount,0,{0,0},wt->clockParams};
    wt->visiblePages[3]={"Inputs",inputCount,0,{0,0},wt->inputParams};
    wt->visiblePages[4]={"Outputs",outputCount,0,{0,0},wt->outputParams};
    wt->visiblePageSet={5,wt->visiblePages};
    wt->parameterPages=&wt->visiblePageSet;
    if(notifyHost){
        const int index=NT_algorithmIndex(wt);
        if(index>=0)NT_updateParameterPages(static_cast<uint32_t>(index));
    }
}
// When the user first advances an optional output from Off, the host bus
// chooser starts at Input 1 (bus 1). Instead, suggest the next unused physical
// output. All channels remain Off by default and arbitrary manual bus routing
// is still possible after the first selection. NT output buses are 13..20.
static int nextFreePhysicalOutput(const ClockedWT* wt,int slot){
    const int first=kNT_numInputBusses+1;
    const int count=kNT_numOutputBusses;
    const int last=first+count-1;
    const int assigned[]={wt->v[kOutput],wt->v[kOutputB],wt->v[kOutputC],wt->v[kOutputD]};
    // Prefer the next output after the previous active channel, then wrap.
    int anchor=assigned[0];
    for(int i=1;i<=slot;++i){
        if(assigned[i]>=first&&assigned[i]<=last)anchor=assigned[i];
    }
    if(anchor<first||anchor>last)anchor=first-1;
    for(int n=0;n<count;++n){
        const int candidate=first+((anchor-first+1+n+count)%count);
        bool used=false;
        for(int j=0;j<4;++j){
            if(j!=slot+1 && assigned[j]==candidate){used=true;break;}
        }
        if(!used)return candidate;
    }
    return 0; // Every physical output is already allocated; keep manual routing.
}
static void parameterChanged(_NT_algorithm* base,int param){
    ClockedWT* wt=static_cast<ClockedWT*>(base);
    if(param==kOutputB||param==kOutputC||param==kOutputD){
        const int slot=(param==kOutputB?0:(param==kOutputC?1:2));
        const int value=wt->v[param];
        // Only intercept the first clockwise increment from Off to Input 1;
        // deliberate manual selection from another bus is never overridden.
        if(wt->previousAuxBus[slot]==0 && value==1){
            const int suggested=nextFreePhysicalOutput(wt,slot);
            if(suggested>0){
                wt->previousAuxBus[slot]=static_cast<int16_t>(suggested);
                const int index=NT_algorithmIndex(wt);
                if(index>=0){
                    NT_setParameterFromUi(static_cast<uint32_t>(index),
                        static_cast<uint32_t>(param)+NT_parameterOffset(),
                        static_cast<int16_t>(suggested));
                }
            }else wt->previousAuxBus[slot]=static_cast<int16_t>(value);
        }else{
            wt->previousAuxBus[slot]=static_cast<int16_t>(value);
        }
    }
    if(param==kClockIn||param==kClockSource||param==kClockFormat||
       (param==kBarsPerPulse&&wt->v[kClockFormat]==kPulseNBar))resetClock(wt);
    if(param==kClockSource||param==kClockFormat||param==kMode||
       param==kOutputB||param==kOutputC||param==kOutputD){
        configurePages(wt,true);
    }
    if(param==kClockSource||param==kClockIn){
        wt->tapSeen=false;
        wt->midiRunning=true;wt->midiTransportSeen=false;
        wt->midiTickCount=0;wt->midiStartCount=0;wt->midiStopCount=0;
    }
    if(param==kTriggerIn){wt->triggerHigh=false;wt->envActive=false;wt->envPhase=0;}
    if(param==kMode){wt->envActive=false;wt->envPhase=0;}
    if(param==kVoltageRange){
        // Offset is relative to the width of the selected range.
        const int maxMv=(wt->v[kVoltageRange]==1)?2500:5000;
        wt->parametersOwned[kOffsetMv].min=static_cast<int16_t>(-maxMv);
        wt->parametersOwned[kOffsetMv].max=static_cast<int16_t>(maxMv);
        int index=NT_algorithmIndex(wt);
        if(index>=0){
            NT_updateParameterDefinition(static_cast<uint32_t>(index),kOffsetMv);
            if(wt->v[kOffsetMv]>maxMv || wt->v[kOffsetMv]<-maxMv){
                const int bounded=wt->v[kOffsetMv]>maxMv?maxMv:-maxMv;
                NT_setParameterFromUi(static_cast<uint32_t>(index),kOffsetMv+NT_parameterOffset(),static_cast<int16_t>(bounded));
            }
        }
    }
    if(param==kBank||param==kMorph||param==kSkew||param==kSquish||
       param==kPhaseOffset||param==kCharacter||param==kAmplitude||
       param==kVoltageRange||param==kOffsetMv)wt->previewValid=false;
    if(param==kOutputB||param==kPhaseB)wt->aux[0].invalidate();
    if(param==kOutputC||param==kPhaseC)wt->aux[1].invalidate();
    if(param==kOutputD||param==kPhaseD)wt->aux[2].invalidate();
    if(param==kBank||param==kMorph||param==kSkew||param==kSquish||param==kPhaseOffset||param==kQuality||param==kCharacter){
        wt->renderValid=false;wt->loFiValid=false;wt->invalidateAux();
    }
}
struct OutputScale {
    float gain;       // wave -> volts
    float activeBase; // offset + nominal center
    float idleBase;   // envelope/gated idle at offset; LFO idle at center
};
static OutputScale outputScale(const ClockedWT* wt){
    const float center=wt->v[kVoltageRange]==0?0.0f:(wt->v[kVoltageRange]==1?2.5f:5.0f);
    const float radius=wt->v[kVoltageRange]==1?2.5f:5.0f;
    const float offset=wt->v[kOffsetMv]*0.001f;
    OutputScale result={radius*clampf(wt->v[kAmplitude]*.01f,0,1),
                        offset+center,offset+(wt->v[kMode]==kLfo?center:0.0f)};
    return result;
}
static float outputFromWave(const OutputScale& scale,float wave,bool active){
    const float volts=active?scale.activeBase+clampf(wave,-1.0f,1.0f)*scale.gain:scale.idleBase;
    return clampf(volts,-10.0f,10.0f);
}
// Amplitude quantization (vertical resolution), not sample-position
// quantization (horizontal resolution). Stepped modes preserve full voltage
// precision and smooth wavetable morphing. Symmetric signed quantization
// preserves exact zero; 2-bit uses a coarse three-level (-1,0,+1) output.
static float quantizeWave(float wave,int character){
    int levels=character==k8Bit?127:
               character==k6Bit?31:character==k4Bit?7:character==k2Bit?1:0;
    if(!levels)return wave;
    float scaled=clampf(wave,-1.0f,1.0f)*levels;
    int value=static_cast<int>(scaled+(scaled>=0.0f?0.5f:-0.5f));
    return static_cast<float>(value)/levels;
}
static float evaluate(const ClockedWT* wt,float phase,float morphCv,float warpCv,float frequency){
    if(!wt->ready||static_cast<int>(wt->request.index)!=wt->v[kBank])return 0;
    _NT_wavetableEvaluation eval={};
    float time=warp(phase,effectiveWarp(wt,warpCv),wt->v[kSquish]*0.01f);
    const int steps=stepCount(wt->v[kCharacter]);
    if(steps>0)
        time=static_cast<float>(static_cast<int>(time*steps))/steps;
    eval.phase=time;
    float scan=clampf(wt->v[kMorph]*0.001f+morphCv*0.2f,0,1);
    uint32_t count=wt->request.numWaves;
    eval.offset=count>1?scan*static_cast<float>(count-1)*(1.0f-0.000001f):0;
    eval.frequency=frequency;
    return NT_evaluateWavetable(wt->request,eval);
}
// Render a phase-shifted copy with its own adaptive/stepped cache.
// This deliberately shares waveform selection, shape and amplitude with A,
// while evaluating only enabled output buses. It does not advance clocks.
static float renderAuxWave(ClockedWT* wt,AuxRender& cache,float phase,float cv,float warpCv,
                           double cycleDelta,int mode){
    const int character=wt->v[kCharacter];
    const int steps=stepCount(character);
    const float hz=static_cast<float>(cycleDelta*NT_globals.sampleRate);
    if(steps>0){
        const float warped=warp(phase,effectiveWarp(wt,warpCv),wt->v[kSquish]*.01f);
        const int bucket=static_cast<int>(warped*steps);
        const float morph=clampf(wt->v[kMorph]*.001f+cv*.2f,0,1);
        const bool morphChanged=
            (morph>cache.lastMorph+.00001f || morph<cache.lastMorph-.00001f);
        if(!cache.heldValid||bucket!=cache.bucket||morphChanged){
            cache.held=evaluate(wt,phase,cv,warpCv,hz);
            cache.bucket=bucket;
            cache.lastMorph=morph;cache.heldValid=true;
        }
        return cache.held;
    }
    if(wt->v[kQuality]==1)return evaluate(wt,phase,cv,warpCv,hz);
    if(!cache.renderValid||cache.remaining<=0){
        const float skew=effectiveWarp(wt,warpCv);
        const float skewRatio=(1.f-skew)/skew;
        const float maxSkew=skewRatio>1.f?skewRatio:1.f/skewRatio;
        const float maxSquish=1.f+.009f*static_cast<float>(wt->v[kSquish]<0?
                                     -wt->v[kSquish]:wt->v[kSquish]);
        const float effectiveHz=hz*maxSkew*maxSquish;
        // Max 32 samples at slow LFO rates: fewer table evaluations with
        // only sub-millisecond Morph CV tracking delay at 48 kHz.
        int interval=32;
        if(effectiveHz>.01f){
            const float ideal=NT_globals.sampleRate/(128.f*effectiveHz);
            if(ideal<32.f)interval=ideal<1.f?1:static_cast<int>(ideal);
        }
        if(mode==kEnvelope && wt->envPhase+cycleDelta*interval>=1.0)interval=1;
        cache.length=interval;cache.remaining=interval;
        cache.waveA=evaluate(wt,phase,cv,warpCv,hz);
        const float future=static_cast<float>(fraction(static_cast<double>(phase)+cycleDelta*interval));
        cache.waveB=interval==1?cache.waveA:evaluate(wt,future,cv,warpCv,hz);
        cache.renderValid=true;
    }
    const float alpha=static_cast<float>(cache.length-cache.remaining)/cache.length;
    --cache.remaining;
    return cache.waveA+(cache.waveB-cache.waveA)*alpha;
}
static void step(_NT_algorithm* base,float* buses,int numFramesBy4){
    ClockedWT* wt=static_cast<ClockedWT*>(base);
    refreshLibrary(wt);
    int frames=numFramesBy4*4;
    if(frames<=0)return;
    int cb=wt->v[kClockIn],tb=wt->v[kTriggerIn],rb=wt->v[kResetIn],mb=wt->v[kMorphIn],wb=wt->v[kWarpIn],ob=wt->v[kOutput];
    const float* clock=cb>0?buses+(cb-1)*frames:nullptr;
    const float* trigger=tb>0?buses+(tb-1)*frames:nullptr;
    const float* reset=rb>0?buses+(rb-1)*frames:nullptr;
    const float* morphCv=mb>0?buses+(mb-1)*frames:nullptr;
    const float* warpCv=wb>0?buses+(wb-1)*frames:nullptr;
    float* out=ob>0?buses+(ob-1)*frames:nullptr;
    // Compile the active phase outputs once per audio block. Off outputs have
    // essentially zero DSP cost and require no dedicated NT algorithm slots.
    static const int kAuxBusParams[3]={kOutputB,kOutputC,kOutputD};
    static const int kAuxPhaseParams[3]={kPhaseB,kPhaseC,kPhaseD};
    float* extraBuffers[3];
    float extraPhases[3];
    int extraIndices[3];
    int numExtras=0;
    for(int n=0;n<3;++n){
        const int bus=wt->v[kAuxBusParams[n]];
        if(bus>0){
            extraBuffers[numExtras]=buses+(bus-1)*frames;
            extraPhases[numExtras]=wt->v[kAuxPhaseParams[n]]*(1.f/360.f);
            extraIndices[numExtras]=n;
            ++numExtras;
        }
    }
    bool replace=wt->v[kOutputMode]!=0;
    int mode=wt->v[kMode],source=wt->v[kClockSource];
    bool external=source==kExternal&&clock!=nullptr;
    bool midi=source==kMidi;
    double rate=ratioAt(wt->v[kRate]);
    double internalPerSample=(wt->v[kInternalBpm]/60.0)/NT_globals.sampleRate;
    const float phOffset=wt->v[kPhaseOffset]*(1.0f/360.0f);
    const OutputScale scale=outputScale(wt);
    // Relative cycle phase is advanced incrementally, avoiding a 64-bit
    // multiply + integer phase reduction for *every sample*. Resynchronize
    // at block boundaries and explicit clock/transport phase jumps.
    double cyclePhase=fraction(wt->unitPosition*rate);
    // Clock interval reciprocal changes on clock edges, not every sample.
    double measuredDelta=wt->hasPeriod?
        (1.0/wt->unitSamples + wt->pllTrimPerSample):internalPerSample;
    for(int i=0;i<frames;++i){
        if(external&&rising(clock[i],wt->clockHigh)){
            clockTick(wt,NT_globals.sampleRate);
            cyclePhase=fraction(wt->unitPosition*rate);
            measuredDelta=wt->hasPeriod?1.0/wt->unitSamples+wt->pllTrimPerSample:internalPerSample;
            wt->renderValid=false;wt->invalidateAux();
        }
        // Tap CV is 1 pulse per quarter note. Use the measured interval DIRECTLY
        // as the clock timebase instead of editing Internal BPM (which may not
        // be applied synchronously by the host). Preserve beat count when
        // tapping repeatedly so divided rates (/4, /8...) can complete cycles.
        if(source==kTap&&clock&&rising(clock[i],wt->clockHigh)) {
            if(!wt->tapSeen){
                wt->tapSeen=true;
                wt->lastTap=wt->samples;
                clockTick(wt,NT_globals.sampleRate,1.0);
            }else{
                uint32_t elapsed=wt->samples-wt->lastTap;
                if(elapsed>=NT_globals.sampleRate/5 &&
                   elapsed<=NT_globals.sampleRate*3U){
                    clockTick(wt,NT_globals.sampleRate,1.0);
                    wt->lastTap=wt->samples;
                }else if(elapsed>NT_globals.sampleRate*3U){
                    // A new tapping session: discard the stale interval.
                    resetClock(wt);
                    wt->lastTap=wt->samples;
                    clockTick(wt,NT_globals.sampleRate,1.0);
                }
                // Pulses closer than 200 ms are ignored as bounce.
            }
            cyclePhase=fraction(wt->unitPosition*rate);
            measuredDelta=wt->hasPeriod?1.0/wt->unitSamples+wt->pllTrimPerSample:0.0;
            wt->renderValid=false;wt->invalidateAux();
        }
        if(reset&&rising(reset[i],wt->resetHigh)){
            wt->unitPosition=0;cyclePhase=0;wt->envActive=false;wt->envPhase=0;wt->renderValid=false;wt->invalidateAux();
            wt->edgeCount=0;wt->clockOriginUnits=0;wt->pllTrimPerSample=0;
        }
        // 1-Bar mode has no knowable tempo until two incoming bar edges.
        // Hold phase during the first bar instead of assuming 4/4.
        // A selected sync source must NEVER masquerade as Internal BPM.
        // External PPQN retains its established fallback for compatibility.
        // Tap and MIDI wait until two actual pulses have established a period.
        double unitDelta=0.0;
        if(source==kInternal)unitDelta=internalPerSample;
        else if(source==kTap || midi)unitDelta=wt->hasPeriod?measuredDelta:0.0;
        else if(external)unitDelta=wt->hasPeriod?measuredDelta:
            ((wt->v[kClockFormat]==kPulseBar||wt->v[kClockFormat]==kPulseNBar)?0.0:internalPerSample);
        if(external&&wt->seenEdge&&wt->hasPeriod){
            double timeout=wt->externalPulsePeriod*2.5;
            // Avoid 24-PPQN clocks freezing from a single missing pulse.
            if(wt->v[kClockFormat]==kPulse24&&timeout<wt->unitSamples)timeout=wt->unitSamples;
            wt->clockStopped=static_cast<double>(wt->samples-wt->lastExternalEdge)>timeout;
        }
        if(external&&wt->clockStopped&&wt->v[kStopMode]==kFreeze)unitDelta=0;
        if(external&&wt->clockStopped&&wt->pllTrimPerSample!=0){
            // Once clock input times out, resume with the last measured
            // tempo but without a stale phase-correction term.
            wt->pllTrimPerSample=0;
            measuredDelta=wt->hasPeriod?1.0/wt->unitSamples:internalPerSample;
        }
        // MIDI always follows transport: Stop and missing ticks freeze the
        // waveform; Start resets phase and Continue resumes the held phase.
        if(midi){
            bool missingTicks=wt->seenEdge&&wt->hasPeriod&&
                static_cast<double>(wt->samples-wt->lastExternalEdge)>
                    wt->externalPulsePeriod*3.0;
            if(!wt->midiRunning||missingTicks){
                wt->pllTrimPerSample=0;
                unitDelta=0; // Transport Stop or missing clock freezes the waveform.
            }
        }
        if(unitDelta<0)unitDelta=0;
        const double cycleDelta=unitDelta*rate;
        bool trigRise=false;
        bool gate=false;
        if(trigger){trigRise=rising(trigger[i],wt->triggerHigh);gate=wt->triggerHigh;}
        float phase=static_cast<float>(cyclePhase);
        bool active=true;
        if(mode==kEnvelope){
            if(trigRise){wt->envActive=true;wt->envPhase=0;wt->renderValid=false;wt->invalidateAux();}
            active=wt->envActive;
            phase=static_cast<float>(clampd(wt->envPhase,0,0.999999));
            if(active){wt->envPhase+=cycleDelta;if(wt->envPhase>=1.0){wt->envPhase=1.0;wt->envActive=false;}}
        } else if(mode==kGated){
            if(trigRise){wt->envPhase=0;wt->envActive=true;wt->renderValid=false;wt->invalidateAux();}
            active=gate;
            phase=static_cast<float>(fraction(wt->envPhase));
            if(active){wt->envPhase+=cycleDelta;if(wt->envPhase>=1.0)wt->envPhase=fraction(wt->envPhase);}
            else wt->envActive=false;
        }
        wt->displayedPhase=phase;
        phase=wrap01(phase+phOffset);
        float cv=morphCv?morphCv[i]:0;
        float warpVoltage=warpCv?warpCv[i]:0;
        wt->lastMorphCv=cv;wt->lastWarpCv=warpVoltage;
        float wave=0;
        bool bankReady=wt->ready&&static_cast<int>(wt->request.index)==wt->v[kBank];
        if(active&&bankReady){
            if(stepCount(wt->v[kCharacter])>0){
                // Sample-and-hold at N phase positions, even in Adaptive mode.
                // Smooth morph interpolation across adjacent waves is retained.
                const int steps=stepCount(wt->v[kCharacter]);
                const float warped=warp(phase,wt->v[kSkew]*0.01f,wt->v[kSquish]*0.01f);
                const int bucket=static_cast<int>(warped*steps);
                const float morphScan=clampf(wt->v[kMorph]*.001f+cv*.2f,0,1);
                const bool changedMorph=
                    (morphScan>wt->loFiMorph+0.00001f||morphScan<wt->loFiMorph-0.00001f);
                if(!wt->loFiValid||bucket!=wt->loFiBucket||changedMorph){
                    wt->loFiHold=evaluate(wt,phase,cv,warpVoltage,static_cast<float>(cycleDelta*NT_globals.sampleRate));
                    wt->loFiBucket=bucket;
                    wt->loFiMorph=morphScan;wt->loFiValid=true;
                }
                wave=wt->loFiHold;
            }else if(wt->v[kQuality]==1){
                // Full-rate reference path, useful for A/B audio and CPU tests.
                wave=evaluate(wt,phase,cv,warpVoltage,static_cast<float>(cycleDelta*NT_globals.sampleRate));
            }else{
                if(!wt->renderValid||wt->renderRemain<=0){
                    // Target >=128 evaluations per effective (phase-warped)
                    // cycle. Strong Skew/Squish increases local phase speed,
                    // so automatically approach full-rate evaluation where needed.
                    // 32-sample upper bound keeps Morph CV latency under 0.67ms at 48 kHz,
                    // while halving the usual wavetable-evaluation count versus v0.3.0.
                    float hz=static_cast<float>(cycleDelta*NT_globals.sampleRate);
                    if(hz<0)hz=0;
                    const float skew=effectiveWarp(wt,warpVoltage);
                    const float skewRatio=(1.f-skew)/skew;
                    const float maxSkewSpeed=skewRatio>1.f?skewRatio:(1.f/skewRatio);
                    const float maxSquishSpeed=1.f+.009f*static_cast<float>(wt->v[kSquish]<0?-wt->v[kSquish]:wt->v[kSquish]);
                    const float effectiveHz=hz*maxSkewSpeed*maxSquishSpeed;
                    int interval=32;
                    if(effectiveHz>0.01f){
                        const float ideal=NT_globals.sampleRate/(128.0f*effectiveHz);
                        if(ideal<32)interval=ideal<1?1:static_cast<int>(ideal);
                    }
                    // Do not interpolate beyond a one-shot envelope's end.
                    if(mode==kEnvelope && wt->envPhase+cycleDelta*interval>=1.0)interval=1;
                    wt->renderLength=interval;
                    wt->renderRemain=interval;
                    wt->waveA=evaluate(wt,phase,cv,warpVoltage,hz);
                    float futurePhase=static_cast<float>(fraction(static_cast<double>(phase)+cycleDelta*interval));
                    wt->waveB=interval==1?wt->waveA:evaluate(wt,futurePhase,cv,warpVoltage,hz);
                    wt->renderValid=true;
                }
                const float alpha=static_cast<float>(wt->renderLength-wt->renderRemain)/wt->renderLength;
                wave=wt->waveA+(wt->waveB-wt->waveA)*alpha;
                --wt->renderRemain;
            }
        }else {wt->renderValid=false;wt->loFiValid=false;}
        wave=quantizeWave(wave,wt->v[kCharacter]);
        float voltage=outputFromWave(scale,wave,active&&bankReady);
        if(out)out[i]=replace?voltage:out[i]+voltage;
        if(numExtras){
            for(int n=0;n<numExtras;++n){
                AuxRender& cache=wt->aux[extraIndices[n]];
                float auxVoltage;
                if(active&&bankReady){
                    const float shifted=wrap01(phase+extraPhases[n]);
                    const float extraWave=quantizeWave(renderAuxWave(wt,cache,shifted,cv,warpVoltage,cycleDelta,mode),wt->v[kCharacter]);
                    auxVoltage=outputFromWave(scale,extraWave,true);
                }else{
                    cache.invalidate();
                    auxVoltage=outputFromWave(scale,0,false);
                }
                float* extra=extraBuffers[n];
                extra[i]=replace?auxVoltage:extra[i]+auxVoltage;
            }
        }
        wt->unitPosition+=unitDelta;
        cyclePhase+=cycleDelta;
        if(cyclePhase>=1.0 || cyclePhase<0.0)cyclePhase=fraction(cyclePhase);
        ++wt->samples;
    }
}
// Construct the realtime sync diagnostic without sprintf/libc dependencies.
// The API calls parameterString on demand while rendering the parameter UI.
static int syncStatus(ClockedWT* wt,char* out){
    const int source=wt->v[kClockSource];
    const char* label="";
    int bpm=0;
    if(source==kInternal){label="Internal ";bpm=wt->v[kInternalBpm];}
    else if(source==kTap){
        if(wt->hasPeriod){label="Tap CV ";bpm=static_cast<int>(NT_globals.sampleRate*60.0/wt->unitSamples+0.5);}
        else label=wt->tapSeen?"Tap: 2nd pulse":"Tap: no pulses";
    }else if(source==kMidi){
        if(!wt->midiRunning && wt->midiTransportSeen)
            label="MIDI: stopped";
        else if(wt->hasPeriod){label="MIDI ";bpm=static_cast<int>(NT_globals.sampleRate*60.0/wt->unitSamples+0.5);}
        else label=wt->midiTickCount?"MIDI: locking":
            (wt->midiStartCount?"MIDI: start only":"MIDI: no ticks");
    }else if(wt->hasPeriod){
        const bool bar=(wt->v[kClockFormat]==kPulseBar || wt->v[kClockFormat]==kPulseNBar);
        // Measured clock duration is samples/quarter for PPQN, samples/bar
        // for bar formats. A quarter-note BPM estimate for bars assumes 4/4.
        // This estimate does NOT influence LFO timing or phase.
        const double quarterUnits=bar?4.0:1.0;
        const double estimated=NT_globals.sampleRate*60.0*quarterUnits/wt->unitSamples;
        bpm=static_cast<int>(estimated+0.5);
        label=bar?"CV ~":"CV ";
    }else label=wt->seenEdge?"CV: 2nd pulse":"CV: no clock";
    int i=0;
    while(label[i] && i<kNT_parameterStringSize-1){out[i]=label[i];++i;}
    if(bpm>0){
        if(bpm>999)bpm=999;
        char digits[4];int n=0;
        do{digits[n++]=static_cast<char>('0'+(bpm%10));bpm/=10;}while(bpm>0&&n<3);
        while(n>0&&i<kNT_parameterStringSize-1)out[i++]=digits[--n];
        if(i<kNT_parameterStringSize-1)out[i++]='b';
        if(i<kNT_parameterStringSize-1)out[i++]='p';
        if(i<kNT_parameterStringSize-1)out[i++]='m';
    }
    out[i]=0;return i;
}
static int parameterString(_NT_algorithm* base,int param,int value,char* buf){
    ClockedWT* wt=static_cast<ClockedWT*>(base);
    if(param==kSyncStatus)return syncStatus(wt,buf);
    if(param!=kBank)return 0;
    const char* label="No wavetables";
    if(!wt->mounted)label="Waiting for SD";
    else if(value>=0&&static_cast<uint32_t>(value)<wt->wavetableCount){
        _NT_wavetableInfo info={};NT_getWavetableInfo(static_cast<uint32_t>(value),info);
        if(info.name)label=info.name;
    }
    int i=0;while(label[i]&&i<kNT_parameterStringSize-1){buf[i]=label[i];++i;}
    buf[i]=0;return i;
}
// All incoming realtime MIDI events can reach a native NT algorithm via
// _NT_factory::midiRealtime. Clock Source = MIDI uses incoming MIDI clock;
// the CV Clock Format parameter is ignored for this source.
static void midiRealtime(_NT_algorithm* base,uint8_t message){
    ClockedWT* wt=static_cast<ClockedWT*>(base);
    if(wt->v[kClockSource]!=kMidi)return;
    switch(message){
        case 0xFA: // MIDI Start: restart from the beginning of the phrase.
            resetClock(wt); // phase = 0, old tempo/PLL measurement invalidated
            wt->midiRunning=true;wt->midiTransportSeen=true;
            ++wt->midiStartCount;
            wt->midiTickCount=0;
            wt->envActive=false;wt->envPhase=0;
            wt->displayedPhase=0;
            wt->invalidateAux();
            break;
        case 0xFB: // Continue: retain current phase
            wt->midiRunning=true;wt->midiTransportSeen=true;
            // Re-anchor incoming tick positions without discarding tempo or
            // the phase maintained while transport was stopped.
            wt->seenEdge=false;wt->edgeCount=0;
            wt->pllTrimPerSample=0;
            break;
        case 0xFC: // Stop: hold phase; Continue later resumes that phase.
            wt->midiRunning=false;wt->midiTransportSeen=true;
            ++wt->midiStopCount;
            wt->pllTrimPerSample=0;
            // Throw away any Adaptive interpolation toward a future phase.
            // The very next block should output the held waveform value.
            wt->renderValid=false;wt->loFiValid=false;wt->invalidateAux();
            break;
        case 0xF8: // MIDI Clock: 24 pulses per quarter-note beat
            ++wt->midiTickCount;
            if(wt->midiRunning){
                clockTick(wt,NT_globals.sampleRate,1.0/24.0);
                wt->renderValid=false;wt->invalidateAux();
            }
            break;
        default:break;
    }
}
static bool draw(_NT_algorithm* base){
    ClockedWT* wt=static_cast<ClockedWT*>(base);
    // Standard parameter line remains at the top (y=0..10).
    NT_drawText(3,12,wt->v[kMode]==kLfo?"LFO":(wt->v[kMode]==kEnvelope?"ENV":"GATE"),11,kNT_textLeft,kNT_textTiny);
    char clockLabel[kNT_parameterStringSize];
    syncStatus(wt,clockLabel);
    NT_drawText(251,12,clockLabel,11,kNT_textRight,kNT_textTiny);
    NT_drawShapeI(kNT_line,4,40,251,40,3);
    if(!wt->ready||static_cast<int>(wt->request.index)!=wt->v[kBank]){
        NT_drawText(80,30,"Loading wavetable",10);return false;
    }
    // Graph maps -10V..+10V to y=58..19; midpoint is y=39.
    // Keep the render samples across screen refreshes unless the wavetable,
    // shaping parameters, Morph CV, or Warp CV actually changes.
    const float morphCv=wt->lastMorphCv,warpCv=wt->lastWarpCv;
    const float morphDiff=morphCv-wt->previewMorphCv;
    const float warpDiff=warpCv-wt->previewWarpCv;
    if(!wt->previewValid || morphDiff>0.005f || morphDiff< -0.005f ||
       warpDiff>0.005f || warpDiff< -0.005f){
        const OutputScale scale=outputScale(wt);
        for(int n=0;n<124;++n){
            const float phase=static_cast<float>(n*2)/248.0f;
            const float sampled=wrap01(phase+wt->v[kPhaseOffset]*(1.0f/360.0f));
            const float wave=quantizeWave(evaluate(wt,sampled,morphCv,warpCv,0.0f),wt->v[kCharacter]);
            const float volts=outputFromWave(scale,wave,true);
            int y=39-static_cast<int>(volts*1.85f);
            if(y<18)y=18;
            if(y>59)y=59;
            wt->previewYs[n]=static_cast<int16_t>(y);
        }
        wt->previewMorphCv=morphCv;
        wt->previewWarpCv=warpCv;
        wt->previewValid=true;
    }
    for(int n=1;n<124;++n)
        NT_drawShapeI(kNT_line,4+(n-1)*2,wt->previewYs[n-1],4+n*2,wt->previewYs[n],15);
    if(wt->v[kPlayhead]){
        int cursor=4+static_cast<int>(wt->displayedPhase*247.0f);
        NT_drawShapeI(kNT_line,cursor,17,cursor,61,6);
    }
    return false;
}
// Keep the host's native pot/page navigation fully in charge on both views.
// A non-null hasCustomUi/customUi callback was only used to capture the
// centre-pot push for Tap Tempo in v0.2.1. It is intentionally disabled here
// to diagnose and restore the stock Left=page, Centre=parameter, Right=value
// navigation in the graphical algorithm UI on NT firmware 1.16.1.
// Tap mode remains usable via incoming pulses at "Clock in"; see step().
static const _NT_factory kFactory={
    NT_MULTICHAR('C','W','T','3'), // incompatible parameter ordering; reject earlier preset mappings
    "Clocked Wavetable", "Clocked wavetable LFO and envelopes",
    0,nullptr,nullptr,nullptr,calculateRequirements,construct,parameterChanged,step,
    draw,midiRealtime,nullptr,kNT_tagUtility,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,parameterString,nullptr
};
} // namespace
extern "C" uintptr_t pluginEntry(_NT_selector selector,uint32_t index){
    switch(selector){
        case kNT_selector_version:return kNT_apiVersion13;
        case kNT_selector_numFactories:return 1;
        case kNT_selector_factoryInfo:return index==0?reinterpret_cast<uintptr_t>(&kFactory):0;
        default:return 0;
    }
}
