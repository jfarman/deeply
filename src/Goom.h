#ifndef STK_GOOM_H
#define STK_GOOM_H

#include "Sampler.h"
#include "FormSwep.h"

namespace stk {
    class Goom : public Sampler
    {
    public:
        //! Class constructor.
        /*!
         An StkError will be thrown if the rawwave path is incorrectly set.
         */
        Goom( void );
        
        //! Class destructor.
        ~Goom( void );
        
        //! Set instrument parameters for a particular frequency.
        void setFrequency( StkFloat frequency );
        
        //! Start a note with the given frequency and amplitude.
        void noteOn( StkFloat frequency, StkFloat amplitude );
        
        //! Set the modulation (vibrato) speed in Hz.
        void setModulationSpeed( StkFloat mSpeed ) { loops_[1]->setFrequency( mSpeed ); };
        
        //! Set the modulation (vibrato) depth.
        void setModulationDepth( StkFloat mDepth ) { modDepth_ = mDepth * 0.5; };
        
        //! Perform the control change specified by \e number and \e value (0.0 - 128.0).
        void controlChange( int number, StkFloat value );
        
        void setAttack(float attack){ adsr.setAttackTime(attack);}
        void setDecay(float decay){adsr.setDecayTime(decay);}
        void setSustain(float sustain){adsr.setSustainLevel(sustain);}
        void setRelease(float release){ adsr.setReleaseTime(release);}
        void setGain(float gain){this->gain = gain;}

        StkFloat tick( unsigned int channel = 0 );

        StkFrames& tick( StkFrames& frames, unsigned int channel = 0 );
        
    protected:
        FormSwep filters_[2];
        StkFloat modDepth_;
        StkFloat filterQ_;
        StkFloat filterRate_;
        
        ADSR adsr;
        float gain;
    };
    
    inline StkFloat Goom :: tick( unsigned int )
    {
        StkFloat temp;
        
        if ( modDepth_ != 0.0 ) {
            temp = loops_[1]->tick() * modDepth_;
            loops_[0]->setFrequency( baseFrequency_ * (1.0 + temp) );
        }
        
        temp = attackGain_ * attacks_[0]->tick();
        temp += loopGain_ * loops_[0]->tick();
        temp = filter_.tick( temp );
        temp *= adsr_.tick();
        temp = filters_[0].tick( temp );
        lastFrame_[0] = filters_[1].tick( temp );
        return lastFrame_[0] * 6.0;
    }
    
    inline StkFrames& Goom :: tick( StkFrames& frames, unsigned int channel )
    {
        unsigned int nChannels = lastFrame_.channels();
#if defined(_STK_DEBUG_)
        if ( channel > frames.channels() - nChannels ) {
            oStream_ << "Moog::tick(): channel and StkFrames arguments are incompatible!";
            handleError( StkError::FUNCTION_ARGUMENT );
        }
#endif
        
        StkFloat *samples = &frames[channel];
        unsigned int j, hop = frames.channels() - nChannels;
        if ( nChannels == 1 ) {
            for ( unsigned int i=0; i<frames.frames(); i++, samples += hop )
                *samples++ = tick();
        }
        else {
            for ( unsigned int i=0; i<frames.frames(); i++, samples += hop ) {
                *samples++ = tick();
                for ( j=1; j<nChannels; j++ )
                    *samples++ = lastFrame_[j];
            }
        }
        
        return frames;
    }
    
} // stk namespace

#endif
