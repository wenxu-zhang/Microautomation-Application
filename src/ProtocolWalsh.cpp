// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Thursday, November 17, 2011</lastedit>
// ===================================
#include "ProtocolWalsh.h"
#include <iostream>
#include <limits>
#include "Ludl.h"
#include <iostream>
#include <windows.h>
#include <vector>
#include "SpotSeriesScan.h"
#include "DefiniteFocus.h"
#include "AndorCam.h"
#include "Math.h"	
#include "FlowChannel.h"
using namespace std;
extern string WORKINGDIR;
extern int protocolIndex;
extern vector<string> protocolFiles;
extern Chamber currentChamber;
extern customProtocol scan;

#ifdef OBSERVER
void ProtocolWalsh::runProtocol(){
	char c;
	while(true){
		try{
		cout<<"Please select a protocol to run:"<<endl;
		cout<<"1: Custom Scan"<<endl;
		cout<<"2: Pump Protocols"<<endl;
		cout<<"3: Chemical Cleavage Reaction"<<endl;
		cout<<"4: Dilution Effect"<<endl;
		cout<<"5: Surface Prep Reactions"<<endl;
		cout<<"6: Incorporation Kinetics"<<endl;
		cout<<"9: Incorporation Kinetics Push"<<endl;
		cout<<"7: in vitro Translation"<<endl;
		cout<<"8: automated functionalization"<<endl;
		cout<<"k: high speed kinetics + pump"<<endl;
		cout<<"8: Pull - automated functionalization"<<endl;
		cout<<"9: Push - automated functionalization"<<endl;
		cout<<"a: Pull - multi-channel automated functionalization"<<endl;
		cout<<"s: Sequencing Cycles"<<endl;
	
		cout<<"e: Exit"<<endl;
		c=::getChar();
		switch(c){
			case '1':
				customScan();
				break;
			case '2':
				pumpProtocols();
				break;
			case '3':
				chemCleave();
				break;
			case '4':
				dilutionEffect();
				break;
			case '5':
				cout<<"select surface prep protocol"<<endl;
				while(true){
					cout<<"1: 65 deg C hybridication"<<endl;
					cout<<"2: Load reagents, hybridize, wash"<<endl;
					cout<<"3: cDNA synthesis with Bst DNAP"<<endl;
					cout<<"e: Exit"<<endl;
					c=::getChar();
					switch(c){
						case '1':
							hybRxn(20,65);
							break;
						case '2':
							loadHyb();
							break;
						case '3':{
							vector<FlowChannel*> f=scan.fs.selectChannels();
							cDNAsynth(f);
							break;}
						case 'e':
							return;
							break;
					}
				}
				break;
			case '6':
				incorpKinetics2();
				break;
			case '7':
				ivTrans();
				break;
			case '8':
				glassFunc();
				break;
			case '9':
				incorpKinetics3();
				break;
			case 'k':
				pumpKinetics();
				break;

			case 'a':
				glassFunc_multiChan();
				break;

			case 's':
				cycleSeq();
				break;

			case 'e':
				return;
				break;

		}
		}catch(std::ios_base::failure e)
			    {
					logFile.write("Protocol Menu Exception: "+string(e.what()),true);
					continue;
				}
		catch(exception& e){
					logFile.write(string(e.what()),true);
					continue;
				}
	}
}
void ProtocolWalsh::totalSurfPrep(){
	//load DNA template and hybridize
	
}
void ProtocolWalsh::glassFunc(){
	char c;
	//Solutions from first daisy valve for reagents
	Solution* f=scan.fs.getSolution("column");
	Solution* w=scan.fs.getSolution("waste");
	Solution* a=scan.fs.getSolution("argon");
	Solution* s=scan.fs.getSolution("silane");
	//Solution* d=scan.fs.getSolution("dmf");
	Solution* p=scan.fs.getSolution("cPEG");
	Solution* n=scan.fs.getSolution("NHS-PEG");
	Solution* t=scan.fs.getSolution("polyT");
	Solution* m=scan.fs.getSolution("HCl");

	//Solutions from second daisy valve - channel selection


	//Flow Channels
	FlowChannel* l=scan.fs.getChannel("loop");
	FlowChannel* h=scan.fs.getChannel("water");
	FlowChannel* e=scan.fs.getChannel("ethanol");
	FlowChannel* b=scan.fs.getChannel("MES");
	FlowChannel* d=scan.fs.getChannel("DMF");
	FlowChannel* ssc=scan.fs.getChannel("SSC");

	cout<<"Select process"<<endl;
	while(true){
		cout<<"1: Activation and aminosilination"<<endl;
		cout<<"2: PEG reactions"<<endl;
		cout<<"3: polyT reaction"<<endl;
		cout<<"4: clean system"<<endl;
		cout<<"5: prime syringe"<<endl;
		cout<<"6: clean syringe and loop line: organics"<<endl;
		cout<<"7: clean syringe and loop line: aqueous"<<endl;
		cout<<"8: NHS peg mod"<<endl;
		cout<<"e:Exit"<<endl;
		c=::getChar();
		switch(c){
		case '1':
			//prime system with water
			h->pull((2*l->injectTubingVolumePull+f->sd.inletTubingVol),100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			//prime HCl soln line
			m->valveSelect();
			l->pull(m->sd.inletTubingVol,m->sd.uLps);
			w->valveSelect();
			l->push(m->sd.inletTubingVol,w->sd.uLps);
			//load HCl soln
			m->valveSelect();
			l->pull(f->sd.inletTubingVol+15,m->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"Acid surface activation start."<<endl;
			//surface activation with acid for 15 minutes
			Timer::wait(15*60*1000);
			cout<<"Acid surface activation done. Wash with dH2O."<<endl;
			//wash with dh2o
			h->pull(f->sd.inletTubingVol*2,w->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);

			//prime syringe and loop line w/ ethanol
			//empty syringe
			e->waste();
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<3; i++)
			{
				e->pull(e->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			e->waste();
			//re-fill syringe with ethanol
			e->pull((2*l->injectTubingVolumePull+f->sd.inletTubingVol),100);
			
			//flush ethanol through device channels
			cout<<"Exchanging ethanol into channels."<<endl;
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			cout<<"Silane solution ready? Press ENTER to continue"<<endl;
			getString();
			//prime silane line
			s->valveSelect();
			l->pull(s->sd.inletTubingVol,s->sd.uLps);
			w->valveSelect();
			l->push(s->sd.inletTubingVol,w->sd.uLps);
			//load silane soln
			s->valveSelect();
			l->pull(f->sd.inletTubingVol+15,s->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"Silane surface activation start."<<endl;
			//silane reaction for 10 minutes
			Timer::wait(10*60*1000);
			cout<<"Silane surface activation end. Wash with ethanol and flush air."<<endl;
			//rinse and dry
			e->pull(f->sd.inletTubingVol*2,w->sd.uLps);
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			a->valveSelect();
			l->pull(f->sd.inletTubingVol*2+30,a->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);

			return;
			break;

		case '2':
			//prime system w/ DMF; empty syringe and load with DMF from loop line
			//empty syringe
			d->waste();
			//wash syringe 1x with DMF
			d->pull(d->s->volume,100);
			d->waste();
			//wash syringe and loop line 2x with DMF
			for (int i=0; i<2; i++)
			{
				d->pull(d->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			w->valveSelect();
			l->waste();
			//re-fill syringe with DMF
			d->pull(3*f->sd.inletTubingVol,100);
			//flush DMF through device channels
			cout<<"Exchanging DMF into channels."<<endl;
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);

			/*l->waste();
			d->valveSelect();
			l->pull(l->injectTubingVolumePull*1.2+d->sd.inletTubingVol,d->sd.uLps);
			//discard syringe of DMF and reload with fresh DMF; prime device channels with DMF
			l->waste();
			d->valveSelect();
			l->pull(3*f->sd.inletTubingVol,d->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);*/

			//prime cPEG soln line
			p->valveSelect();
			l->pull(p->sd.inletTubingVol,p->sd.uLps);
			w->valveSelect();
			l->push(p->sd.inletTubingVol,w->sd.uLps);
			//load cPEG soln
			p->valveSelect();
			l->pull(f->sd.inletTubingVol+15,p->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"cPEG rxn"<<endl;
			Timer::wait(120*60*1000);//react for 2 hours
			cout<<"cPEG rxn complete."<<endl;
			//rinse channels 1x with DMF in syringe
			f->valveSelect();
			l->push(f->sd.inletTubingVol*1.2,f->sd.uLps);
			cout<<"Please proceed to NHS-PEG reaction steps."<<endl;
			
			return;
			break;
		case '3':
			//prime solution w/ aqueous buffer
			//empties syringe
			h->waste();
			//wash syringe 1x with dh2o
			h->pull(h->s->volume,100);
			h->waste();
			//wash syringe and loop line 3x with dh2o
			for (int i=0; i<3; i++)
			{
				h->pull(h->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			h->waste();
			//fill syringe with MES buffer and rinse syringe and loop line with buffer 2x
			for (int i=0; i<2; i++)
			{
				b->pull(b->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			b->pull((2*l->injectTubingVolumePull+f->sd.inletTubingVol),100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps); //channels filled w/ buffer

			//prime polyT soln line
			cout<<"Prepare polyT rxn mix. Press ENTER when ready to continue"<<endl;
			getString();
			t->valveSelect();
			l->pull(t->sd.inletTubingVol,t->sd.uLps);
			w->valveSelect();
			l->push(t->sd.inletTubingVol,w->sd.uLps);
			//load polyT soln
			t->valveSelect();
			l->pull(f->sd.inletTubingVol+15,t->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"polyT immobilization rxn"<<endl;
			//react 2 hours
			Timer::wait(120*60*1000);
			cout<<"polyT rxn complete"<<endl;
					
			//wash syringe and loop line 3x with dh2o
			for (int i=0; i<3; i++)
			{
				h->pull(h->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			h->waste();
			//fill syringe with SSC buffer and rinse syringe and loop line with buffer 2x
			for (int i=0; i<2; i++)
			{
				ssc->pull(ssc->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			ssc->pull((2*l->injectTubingVolumePull+f->sd.inletTubingVol),100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps); //channels rinsed w/ buffer
			return;
			break;

		case '4': //clean
			cout<<"put tubes 4-8 in 0.1 M NaOH. Press ENTER when done."<<endl;
			getString();
			//vector<Solutions*> q, would like to make vector of channels to pull and iterate through them w/ for loop
			s->valveSelect();
			l->pull(150,30);
			//d->valveSelect();
			//l->pull(150,30);
			p->valveSelect();
			l->pull(150,30);
			n->valveSelect();
			l->pull(150,30);
			t->valveSelect();
			l->pull(150,30);

			cout<<"put tubes 4-8 in 1% SDS. Press ENTER when done."<<endl;
			getString();
			s->valveSelect();
			l->pull(150,30);
			//d->valveSelect();
			//l->pull(150,30);
			p->valveSelect();
			l->pull(150,30);
			n->valveSelect();
			l->pull(150,30);
			t->valveSelect();
			l->pull(150,30);

			cout<<"put tubes 4-8 in water. Press ENTER when done."<<endl;
			getString();
			s->valveSelect();
			l->pull(1000,100);
			//d->valveSelect();
			//l->pull(1000,100);
			p->valveSelect();
			l->pull(1000,100);
			n->valveSelect();
			l->pull(1000,100);
			t->valveSelect();
			l->pull(1000,100);
			return;
			break;

		case '5': //prime solutions on syringe
			h->pull(h->injectTubingVolumePull,w->sd.uLps);
			b->pull(b->injectTubingVolumePull,w->sd.uLps);
			e->pull(e->injectTubingVolumePull,w->sd.uLps);
			d->pull(d->injectTubingVolumePull,w->sd.uLps);
			ssc->pull(ssc->injectTubingVolumePull,w->sd.uLps);
			return;
			break;

		case '6':
			//empty syringe
			e->waste();
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<3; i++)
			{
				e->pull(e->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			e->waste();
			return;
			break;

		case '7':
			//empties syringe
			h->waste();
			//wash syringe 1x with dh2o
			h->pull(h->s->volume,100);
			h->waste();
			//wash syringe and loop line 3x with dh2o
			for (int i=0; i<3; i++)
			{
				h->pull(h->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			h->waste();
			return;
			break;

		case '8':
			//prime NHS-PEG 
			n->valveSelect();
			l->pull(n->sd.inletTubingVol,n->sd.uLps);
			w->valveSelect();
			l->push(n->sd.inletTubingVol,w->sd.uLps);
			//load NHS-PEG
			n->valveSelect();
			l->pull(f->sd.inletTubingVol+15,n->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"NHS-PEG blocking rxn start."<<endl;
			Timer::wait(60*60*1000);//react for 1 hour
			cout<<"NHS-PEG blocking rxn end."<<endl;
			//wash with DMF, then with ethanol
			d->pull(f->sd.inletTubingVol,100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			/*d->valveSelect();
			l->pull(f->sd.inletTubingVol,d->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);*/
			//empty syringe and line of DMF, fill with ethanol
			e->waste();
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<3; i++)
			{
				e->pull(e->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			//refill syringe with ethanol and wash device channels 2x
			e->pull(f->sd.inletTubingVol*2,100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			//run air through channels to dry
			a->valveSelect();
			l->pull(f->sd.inletTubingVol*2+30,a->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);

			return;
			break;
		case 'e':
			return;
			break;
		}
		
	}

}

void ProtocolWalsh::glassFunc_multiChan(){
	/*
	char j;
	double vol;
	int totinletvol=0;
	vector<Solution*> v;
	//Solutions from first daisy valve for reagents
	Solution* ch1=scan.fs.getSolution("ch1");
	Solution* ch2=scan.fs.getSolution("ch2");
	Solution* ch3=scan.fs.getSolution("ch3");
	Solution* ch4=scan.fs.getSolution("ch4");
	Solution* ch5=scan.fs.getSolution("ch5");
	Solution* ch6=scan.fs.getSolution("ch6");
	Solution* ch7=scan.fs.getSolution("ch7");
	Solution* ch8=scan.fs.getSolution("ch8");
	Solution* w_ch=scan.fs.getSolution("ch_waste");
	Solution* w=scan.fs.getSolution("waste");
	Solution* a=scan.fs.getSolution("argon");
	Solution* s=scan.fs.getSolution("silane");
	//Solution* d=scan.fs.getSolution("dmf");
	Solution* p=scan.fs.getSolution("cPEG");
	Solution* n=scan.fs.getSolution("NHS-PEG");
	Solution* t=scan.fs.getSolution("polyT");
	Solution* m=scan.fs.getSolution("HCl");

	//Solutions from second daisy valve - channel selection


	//Flow Channels
	FlowChannel* l=scan.fs.getChannel("loop");
	FlowChannel* h=scan.fs.getChannel("water");
	FlowChannel* e=scan.fs.getChannel("ethanol");
	FlowChannel* b=scan.fs.getChannel("MES");
	FlowChannel* d=scan.fs.getChannel("DMF");
	FlowChannel* ssc=scan.fs.getChannel("SSC");
	FlowChannel* naoh=scan.fs.getChannel("NaOH");
	FlowChannel* sds=scan.fs.getChannel("SDS");

	cout<<"Select process"<<endl;
	while(true){
		cout<<"1: Activation and aminosilination"<<endl;
		cout<<"2: PEG reactions"<<endl;
		cout<<"3: polyT reaction"<<endl;
		cout<<"4: clean system"<<endl;
		cout<<"5: prime syringe"<<endl;
		cout<<"6: clean syringe and loop line: organics"<<endl;
		cout<<"7: clean syringe and loop line: aqueous"<<endl;
		cout<<"8: NHS peg mod"<<endl;
		cout<<"9: Pre-clean device and channels"<<endl;
		cout<<"e: Exit"<<endl;
		j=::getChar();
		switch(j) {
		case '1':
			cout<<"Select channels to perform activation and silanization."<<endl;
			v=scan.fs.selectSolutions();

			//prime system line with water
			h->pull(l->s->volume,w_ch->sd.uLps);
			l->s->waste();		//wash syringe
			h->pull(l->s->volume,w_ch->sd.uLps);
			w_ch->valveSelect();	//wash line
			l->push(l->s->volume,w_ch->sd.uLps);

			//prime channels w/ water
			h->pull(l->s->volume,w_ch->sd.uLps);
			vol=l->s->volume/v.size();	//divides syringe volume by number of channels to prime
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
				(*i)->valveSelect();			//iterate through channels and prime
				l->push(vol,(*i)->sd.uLps);
				totinletvol+=(*i)->sd.inletTubingVol+30;
			}
			//prime HNO3 soln line
			h->pull(ch1->sd.dv.daisyInjectTubingVol+50,w->sd.uLps);
			m->valveSelect();
			l->pull(m->sd.inletTubingVol,m->sd.uLps);
			w_ch->valveSelect();
			l->push(m->sd.inletTubingVol+ch1->sd.dv.daisyInjectTubingVol+10,w_ch->sd.uLps);
			m->valveSelect();
			//load with HNO3, enough for all channels
			l->pull(totinletvol,m->sd.uLps);
			w_ch->valveSelect();
			//push HNO3 into daisy valve
			l->push(ch1->sd.dv.daisyInjectTubingVol,w_ch->sd.uLps);
			
			//load HCl soln into every channel
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){	//iterate through channels and load HNO3, chase with water
				//m->valveSelect();
				//l->pull(30,m->sd.uLps);
				//w_ch->valveSelect();
				//h->pull((*i)->sd.inletTubingVol-15+(*i)->sd.dv.daisyInjectTubingVol,100);
				//l->push((*i)->sd.dv.daisyInjectTubingVol,w_ch->sd.uLps);
				//(*i)->valveSelect();			
				//l->push(30+(*i)->sd.inletTubingVol-15,(*i)->sd.uLps);
				
				(*i)->valveSelect();			
				l->push((*i)->sd.inletTubingVol+20,(*i)->sd.uLps);
			}
			cout<<"Acid surface activation start."<<endl;
			//surface activation with acid for 15 minutes
			Timer::wait(15*60*1000);
			cout<<"Acid surface activation done. Wash with dH2O."<<endl;
			//wash with dh2o
			h->waste();
			h->pull(l->s->volume,w_ch->sd.uLps);
			w_ch->valveSelect();
			l->push(l->s->volume,w_ch->sd.uLps);
			for (int j=0; j<3; j++)
			{
				h->pull(l->s->volume,w_ch->sd.uLps);
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
					(*i)->valveSelect();			//iterate through channels and prime
					l->push(vol,(*i)->sd.uLps);
				}
			}
			//prime syringe and loop line w/ ethanol
			//empty syringe
			e->waste();
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<3; i++)
			{
				e->pull(e->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			//wash channels with EtOH
			cout<<"Exchanging ethanol into channels."<<endl;
			e->pull(l->s->volume,w_ch->sd.uLps);
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
				(*i)->valveSelect();			//iterate through channels and prime
				l->push(vol,(*i)->sd.uLps);
			}
			
			cout<<"Silane solution ready? Press ENTER to continue"<<endl;
			getString();
			//prime silane line
			e->pull(ch1->sd.dv.daisyInjectTubingVol+50,w->sd.uLps);
			s->valveSelect();
			l->pull(s->sd.inletTubingVol,s->sd.uLps);
			w_ch->valveSelect();
			l->push(s->sd.inletTubingVol+ch1->sd.dv.daisyInjectTubingVol+10,w_ch->sd.uLps);
			s->valveSelect();
			//load with silane, enough for all channels
			l->pull(totinletvol,s->sd.uLps);
			w_ch->valveSelect();
			//push silane into daisy valve
			l->push(ch1->sd.dv.daisyInjectTubingVol,w_ch->sd.uLps);

			//load silane soln
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){	
				//s->valveSelect();
				//l->pull(30,s->sd.uLps);
				//w_ch->valveSelect();
				//e->pull((*i)->sd.inletTubingVol-15+(*i)->sd.dv.daisyInjectTubingVol,100);
				//l->push((*i)->sd.dv.daisyInjectTubingVol,w_ch->sd.uLps);
				//(*i)->valveSelect();			
				//l->push(30+(*i)->sd.inletTubingVol-15,(*i)->sd.uLps);
				(*i)->valveSelect();			
				l->push(20+(*i)->sd.inletTubingVol,(*i)->sd.uLps);
			}
			cout<<"Silane surface activation start."<<endl;
			//silane reaction for 10 minutes
			Timer::wait(10*60*1000);
			cout<<"Silane surface activation end. Wash with ethanol and flush air."<<endl;
			//rinse and dry
			
			e->waste();
			e->pull(l->s->volume,w_ch->sd.uLps);
			w_ch->valveSelect();
			l->push(l->s->volume,w_ch->sd.uLps);
			for (j=0; j<3; j++)
			{
				e->pull(l->s->volume,w_ch->sd.uLps);
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){		//wash channels with EtOH
					(*i)->valveSelect();			
					l->push(vol,(*i)->sd.uLps);
				}
			}
			//dump syringe and fill with air
			e->waste();
			a->valveSelect();
			l->pull(l->s->volume,a->sd.uLps);
			w_ch->valveSelect();
			l->push(l->s->volume,a->sd.uLps);
			a->valveSelect();
			l->pull(l->s->volume,a->sd.uLps);
			e->waste();
			//load air into daisy valve, just before channels
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){		//wash channels with air
				a->valveSelect();
				l->pull(l->s->volume,a->sd.uLps);
				(*i)->valveSelect();			
				l->push(l->s->volume,(*i)->sd.uLps);
				
			}
			
			return;
			break;
			
		case '2':
			totinletvol=0;
			
			cout<<"Select channels to perform COOH-PEG functionalization."<<endl;
			v=scan.fs.selectSolutions();
			vol=l->s->volume/v.size();	//divides syringe volume by number of channels to prime
			//prime system w/ DMF; empty syringe and load with DMF from loop line
			//empty syringe
			d->waste();
			//wash syringe 1x with DMF
			d->pull(d->s->volume,100);
			d->waste();
			//wash syringe and loop line 2x with DMF
			for (int i=0; i<2; i++)
			{
				d->pull(d->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			d->waste();
			//re-fill syringe with DMF
			d->pull(l->s->volume,100);
			//flush DMF through device channels
			cout<<"Exchanging DMF into channels."<<endl;
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){		//wash channels with DMF
				(*i)->valveSelect();			
				l->push(vol,(*i)->sd.uLps);
				totinletvol+=(*i)->sd.inletTubingVol+30;
			}
			
			//prime cPEG soln line
			d->pull(ch1->sd.dv.daisyInjectTubingVol+50,w->sd.uLps);
			p->valveSelect();
			l->pull(m->sd.inletTubingVol,m->sd.uLps);
			w_ch->valveSelect();
			l->push(m->sd.inletTubingVol+ch1->sd.dv.daisyInjectTubingVol+10,w_ch->sd.uLps);
			p->valveSelect();
			//load with cPEG, enough for all channels
			l->pull(totinletvol,m->sd.uLps);
			w_ch->valveSelect();
			//push cPEG into daisy valve
			l->push(ch1->sd.dv.daisyInjectTubingVol,w_ch->sd.uLps);
		
			//load cPEG soln
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){	//iterate through channels and load cPEG, chase with DMF
				(*i)->valveSelect();			
				l->push((*i)->sd.inletTubingVol+20,(*i)->sd.uLps);
			}
			cout<<"cPEG rxn"<<endl;
			Timer::wait(120*60*1000);//react for 2 hours
			cout<<"cPEG rxn complete."<<endl;
			//rinse channels with DMF
			d->waste();
			d->pull(l->s->volume,w_ch->sd.uLps);
			w_ch->valveSelect();
			l->push(l->s->volume,w_ch->sd.uLps);
			for (int j=0; j<3; j++)
			{
				d->pull(l->s->volume,w_ch->sd.uLps);
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
					(*i)->valveSelect();			
					l->push(vol,(*i)->sd.uLps);
				}
			}
			cout<<"Please proceed to NHS-PEG reaction steps."<<endl;
			
			return;
			break;
		case '3':
			totinletvol=0;
			
			cout<<"Select channels to perform poly-T conjugation."<<endl;
			v=scan.fs.selectSolutions();
			vol=l->s->volume/v.size();	//divides syringe volume by number of channels to prime
			//prime syringe w/ MES buffer
			//empties syringe
			h->waste();
			//wash syringe 1x with dh2o
			h->pull(h->s->volume,100);
			h->waste();
			//wash syringe and loop line 3x with dh2o
			for (int i=0; i<3; i++)
			{
				h->pull(h->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			h->waste();
			//fill syringe with MES buffer and rinse syringe and loop line with buffer 2x
			for (int i=0; i<2; i++)
			{
				b->pull(b->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			cout<<"Exchanging MES into channels."<<endl;
			b->pull(l->s->volume,w_ch->sd.uLps);
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
				(*i)->valveSelect();			//iterate through channels and prime
				l->push(vol,(*i)->sd.uLps);
				totinletvol+=(*i)->sd.inletTubingVol+30;
			}
			
			//prime polyT soln line
			cout<<"Prepare polyT rxn mix. Press ENTER when ready to continue"<<endl;
			getString();
			b->pull(ch1->sd.dv.daisyInjectTubingVol+50,w->sd.uLps);
			t->valveSelect();
			l->pull(s->sd.inletTubingVol,s->sd.uLps);
			w_ch->valveSelect();
			l->push(s->sd.inletTubingVol+ch1->sd.dv.daisyInjectTubingVol+10,w_ch->sd.uLps);
			t->valveSelect();
			//load with polyT, enough for all channels
			l->pull(totinletvol,s->sd.uLps);
			w_ch->valveSelect();
			//push polyT into daisy valve
			l->push(ch1->sd.dv.daisyInjectTubingVol,w_ch->sd.uLps);
			//load polyT soln into channels
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){	
				(*i)->valveSelect();			
				l->push(20+(*i)->sd.inletTubingVol,(*i)->sd.uLps);
			}
			cout<<"polyT immobilization rxn"<<endl;
			//react 2 hours
			Timer::wait(120*60*1000);
			cout<<"polyT rxn complete"<<endl;
				
			//wash syringe and loop line 3x with dh2o
			for (int i=0; i<3; i++)
			{
				h->pull(h->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			h->waste();
			//fill syringe with SSC buffer and rinse syringe and loop line with buffer 2x
			for (int i=0; i<2; i++)
			{
				ssc->pull(ssc->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			//flush channels with SSC, 3x
			cout<<"Exchanging SSC into channels."<<endl;
			for (int j=0; j<3; j++)
			{
				ssc->pull(l->s->volume,w_ch->sd.uLps);
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
					(*i)->valveSelect();			//iterate through channels and wash
					l->push(vol,(*i)->sd.uLps);
				}
			}
			return;
			break;

		case '4': //clean
			cout<<"put tubes 4, 5, 7, and 8 in 0.1 M NaOH. Press ENTER when done."<<endl;
			getString();
			//vector<Solutions*> q, would like to make vector of channels to pull and iterate through them w/ for loop
			s->valveSelect();
			l->pull(150,30);
			p->valveSelect();
			l->pull(150,30);
			n->valveSelect();
			l->pull(150,30);
			t->valveSelect();
			l->pull(150,30);

			cout<<"put tubes 4, 5, 7, and 8 in 1% SDS. Press ENTER when done."<<endl;
			getString();
			s->valveSelect();
			l->pull(150,30);
			p->valveSelect();
			l->pull(150,30);
			n->valveSelect();
			l->pull(150,30);
			t->valveSelect();
			l->pull(150,30);

			cout<<"put tubes 4, 5, 7, and 8 in water. Press ENTER when done."<<endl;
			getString();
			s->valveSelect();
			l->pull(1000,100);
			p->valveSelect();
			l->pull(1000,100);
			n->valveSelect();
			l->pull(1000,100);
			t->valveSelect();
			l->pull(1000,100);
			return;
			break;

		case '5': //prime solutions on syringe
			h->pull(h->injectTubingVolumePull,w->sd.uLps);
			b->pull(b->injectTubingVolumePull,w->sd.uLps);
			e->pull(e->injectTubingVolumePull,w->sd.uLps);
			d->pull(d->injectTubingVolumePull,w->sd.uLps);
			ssc->pull(ssc->injectTubingVolumePull,w->sd.uLps);
			naoh->pull(naoh->injectTubingVolumePull,w->sd.uLps);
			sds->pull(sds->injectTubingVolumePull,w->sd.uLps);
			return;
			break;
			
		case '6':
			//empty syringe
			e->waste();
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<3; i++)
			{
				e->pull(e->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			e->waste();
			return;
			break;

		case '7':
			//empties syringe
			h->waste();
			//wash syringe 1x with dh2o
			h->pull(h->s->volume,100);
			h->waste();
			//wash syringe and loop line 3x with dh2o
			for (int i=0; i<3; i++)
			{
				h->pull(h->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			h->waste();
			return;
			break;
		
		case '8':
			totinletvol=0;
			
			cout<<"Select channels to perform NHS-PEG functionalization."<<endl;
			v=scan.fs.selectSolutions();
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){		//calculate totinletvol
				totinletvol+=(*i)->sd.inletTubingVol+30;
			}
			vol=l->s->volume/v.size();	//divides syringe volume by number of channels to prime
			//prime NHS-PEG 
			d->pull(ch1->sd.dv.daisyInjectTubingVol+50,w->sd.uLps);
			n->valveSelect();
			l->pull(m->sd.inletTubingVol,m->sd.uLps);
			w_ch->valveSelect();
			l->push(m->sd.inletTubingVol+ch1->sd.dv.daisyInjectTubingVol+10,w_ch->sd.uLps);
			n->valveSelect();
			//load with NHS-PEG, enough for all channels
			l->pull(totinletvol,m->sd.uLps);
			w_ch->valveSelect();
			//push NHS-PEG into daisy valve
			l->push(ch1->sd.dv.daisyInjectTubingVol,w_ch->sd.uLps);
		
			//load NHS-PEG soln
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){	//iterate through channels and load NHS-PEG, chase with DMF
				(*i)->valveSelect();			
				l->push((*i)->sd.inletTubingVol+20,(*i)->sd.uLps);
			}
			cout<<"NHS-PEG blocking rxn start."<<endl;
			Timer::wait(60*60*1000);//react for 1 hour
			cout<<"NHS-PEG blocking rxn end."<<endl;
			//wash with DMF, then with ethanol
			d->waste();
			d->pull(l->s->volume,w_ch->sd.uLps);
			w_ch->valveSelect();
			l->push(l->s->volume,w_ch->sd.uLps);
			d->pull(l->s->volume,w_ch->sd.uLps);
			//wash w/ DMF, 1x
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
				(*i)->valveSelect();			
				l->push(vol,(*i)->sd.uLps);
			}
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<3; i++)
			{
				e->pull(e->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			//refill syringe with ethanol and wash device channels, 3x
			for (int j=0; j<3; j++)
			{
				e->pull(l->s->volume,w_ch->sd.uLps);
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
					(*i)->valveSelect();			
					l->push(vol,(*i)->sd.uLps);
				}
			}
			//run air through channels to dry
			//dump syringe and fill with air
			e->waste();
			a->valveSelect();
			l->pull(l->s->volume,a->sd.uLps);
			w_ch->valveSelect();
			l->push(l->s->volume,a->sd.uLps);
			a->valveSelect();
			l->pull(l->s->volume,a->sd.uLps);
			e->waste();
			//load air into daisy valve, just before channels
			for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){		//wash channels with air
				a->valveSelect();
				l->pull(l->s->volume,a->sd.uLps);
				(*i)->valveSelect();			
				l->push(l->s->volume,(*i)->sd.uLps);		
			}
			return;
			break;
			
			case '9':	
			//This clean flushes the syringe, loop line, and channels with NaOH, then SDS, and finally dH2O.
			//The channels are flushed serially multiple times, with a 30 second hold between each cycle to allow diffusion of debris
			//from the surface up into the bulk where it can be rinsed away.
			
			cout<<"Select channels to perform pre-cleaning."<<endl;
			v=scan.fs.selectSolutions();
			
			//rinse syringe with NaOH, 2x
			naoh->pull(l->s->volume,w_ch->sd.uLps);
			h->waste();	
			naoh->pull(l->s->volume,w_ch->sd.uLps);
			h->waste();
			//wash syringe and loop line 3x with NaOH
			for (int i=0; i<3; i++)
			{
				naoh->pull(e->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			//wash channels w/ NaOH; 3 times
			vol=l->s->volume/v.size();	//divides syringe volume by number of channels to prime
			for (int j=0; j<3; j++)
			{
				naoh->pull(l->s->volume,w_ch->sd.uLps);
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
					(*i)->valveSelect();			//iterate through channels and flush
					l->push(vol,(*i)->sd.uLps);
				}
				Timer::wait(10*60*1000);//wait 30 seconds
			}
			//wash syringe and line with dH2O 1x
			h->pull(l->s->volume,w_ch->sd.uLps);
			h->waste();	
			h->pull(l->s->volume,w_ch->sd.uLps);
			w_ch->valveSelect();
			l->push(l->s->volume,w_ch->sd.uLps);

			//wash syringe and line with SDS, 1x
			sds->pull(l->s->volume,w_ch->sd.uLps);
			sds->waste();	
			sds->pull(l->s->volume,w_ch->sd.uLps);
			w_ch->valveSelect();
			l->push(l->s->volume,w_ch->sd.uLps);
			//wash channels w/ SDS; 3 times
			for (int j=0; j<3; j++)
			{
				sds->pull(l->s->volume,w_ch->sd.uLps);
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
					(*i)->valveSelect();			//iterate through channels and prime
					l->push(vol,(*i)->sd.uLps);
				}
				Timer::wait(0.5*60*1000);//wait 30 seconds
			}
			//wash syringe and line with dH2O 
			h->pull(l->s->volume,w_ch->sd.uLps);
			h->waste();	
			for (int i=0; i<3; i++)
			{
				h->pull(e->s->volume,100);
				w_ch->valveSelect(); 
				l->push(l->s->volume,w_ch->sd.uLps);
			}
			//wash channels w/ H2O; 5 times
			for (int j=0; j<5; j++)
			{
				h->pull(l->s->volume,w_ch->sd.uLps);
				for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
					(*i)->valveSelect();			//iterate through channels and prime
					l->push(vol,(*i)->sd.uLps);
				}
				Timer::wait(0.5*60*1000);//wait 30 seconds
			}
			return;
			break;

		case 'e':
			return;
			break;
		}
		
	}
*/
}
/*
void ProtocolWalsh::glassFunc_Push(){
	char c;
	//Solutions
	Solution* h=scan.fs.getSolution("water");
	Solution* e=scan.fs.getSolution("ethanol");
	Solution* b=scan.fs.getSolution("MES");
	Solution* w=scan.fs.getSolution("waste");
	Solution* a=scan.fs.getSolution("argon");
	Solution* d=scan.fs.getSolution("dmf");
	//Reagents
	Solution* s=scan.fs.getReagent("silane");
	Solution* p=scan.fs.getReagent("cPEG");
	Solution* n=scan.fs.getReagent("NHS-PEG");
	Solution* t=scan.fs.getReagent("polyT");
	Solution* m=scan.fs.getReagent("HCl");

	//Flow Channels
	FlowChannel* f=scan.fs.getChannel("column");

	cout<<"Select process"<<endl;
	while(true){
		cout<<"1: Activation and aminosilination"<<endl;
		cout<<"2: PEG reactions"<<endl;
		cout<<"3: polyT reaction"<<endl;
		cout<<"4: clean system"<<endl;
		cout<<"5: prime syringe"<<endl;
		cout<<"6: clean syringe and loop line: organics"<<endl;
		cout<<"7: clean syringe and loop line: aqueous"<<endl;
		cout<<"8: NHS peg mod"<<endl;
		cout<<"e:Exit"<<endl;
		c=::getChar();
		switch(c){
		case '1':
			//prime system with water
			h->valveSelect
			

			h->pull((2*l->injectTubingVolumePull+f->sd.inletTubingVol),100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			//prime HCl soln line
			m->valveSelect();
			l->pull(m->sd.inletTubingVol,m->sd.uLps);
			w->valveSelect();
			l->push(m->sd.inletTubingVol,w->sd.uLps);
			//load HCl soln
			m->valveSelect();
			l->pull(f->sd.inletTubingVol+15,m->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"Acid surface activation start."<<endl;
			//surface activation with acid for 15 minutes
			Timer::wait(15*60*1000);
			cout<<"Acid surface activation done. Wash with dH2O."<<endl;
			//wash with dh2o
			h->pull(f->sd.inletTubingVol*2,w->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);

			//prime syringe and loop line w/ ethanol
			//empty syringe
			e->waste();
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<1; i++)
			{
				e->pull(e->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			e->waste();
			//re-fill syringe with ethanol
			e->pull((2*l->injectTubingVolumePull+f->sd.inletTubingVol),100);
			
			//flush ethanol through device channels
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			//prime silane line
			s->valveSelect();
			l->pull(s->sd.inletTubingVol,s->sd.uLps);
			w->valveSelect();
			l->push(s->sd.inletTubingVol,w->sd.uLps);
			//load silane soln
			s->valveSelect();
			l->pull(f->sd.inletTubingVol+15,s->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"Silane surface activation start."<<endl;
			//silane reaction for 10 minutes
			Timer::wait(10*60*1000);
			cout<<"Silane surface activation end. Wash with ethanol and flush air."<<endl;
			//rinse and dry
			e->pull(f->sd.inletTubingVol*2,w->sd.uLps);
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			a->valveSelect();
			l->pull(f->sd.inletTubingVol*2+15,a->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);

			return;
			break;

		case '2':
			//prime system w/ DMF; empty syringe and load with DMF from loop line
			l->waste();
			d->valveSelect();
			l->pull(l->injectTubingVolumePull*1.2+d->sd.inletTubingVol,d->sd.uLps);
			//discard syringe of DMF and reload with fresh DMF; prime device channels with DMF
			l->waste();
			d->valveSelect();
			l->pull(3*f->sd.inletTubingVol,d->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			//prime cPEG soln line
			p->valveSelect();
			l->pull(p->sd.inletTubingVol,p->sd.uLps);
			w->valveSelect();
			l->push(p->sd.inletTubingVol,w->sd.uLps);
			//load cPEG soln
			p->valveSelect();
			l->pull(f->sd.inletTubingVol+15,p->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"cPEG rxn"<<endl;
			Timer::wait(120*60*1000);//react for 2 hours
			cout<<"cPEG rxn complete."<<endl;
			//rinse channels 1x with DMF in syringe
			f->valveSelect();
			l->push(f->sd.inletTubingVol*1.2,f->sd.uLps);
			cout<<"Please proceed to NHS-PEG reaction steps."<<endl;
			
			return;
			break;
		case '3':
			//prime solution w/ aqueous buffer
			//empties syringe
			h->waste();
			//wash syringe 1x with dh2o
			h->pull(h->s->volume,100);
			h->waste();
			//wash syringe and loop line 3x with dh2o
			for (int i=0; i<3; i++)
			{
				h->pull(h->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			h->waste();
			//fill syringe with MES buffer and rinse syringe and loop line with buffer 2x
			for (int i=0; i<2; i++)
			{
				b->pull(b->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			b->pull((2*l->injectTubingVolumePull+f->sd.inletTubingVol),100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps); //channels filled w/ buffer

			//prime polyT soln line
			cout<<"Prepare polyT rxn mix. Press ENTER when ready to continue"<<endl;
			getString();
			t->valveSelect();
			l->pull(t->sd.inletTubingVol,t->sd.uLps);
			w->valveSelect();
			l->push(t->sd.inletTubingVol,w->sd.uLps);
			//load polyT soln
			t->valveSelect();
			l->pull(f->sd.inletTubingVol+15,t->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"polyT immobilization rxn"<<endl;
			//react 2 hours
			Timer::wait(1*60*1000);
			cout<<"polyT rxn complete"<<endl;
			//wash device channels with MES buffer
			b->pull(f->sd.inletTubingVol*2, 100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			return;
			break;

		case '4': //clean
			cout<<"put tubes 4-8 in 0.1 M NaOH. Press ENTER when done."<<endl;
			getString();
			//vector<Solutions*> q, would like to make vector of channels to pull and iterate through them w/ for loop
			s->valveSelect();
			l->pull(150,30);
			d->valveSelect();
			l->pull(150,30);
			p->valveSelect();
			l->pull(150,30);
			n->valveSelect();
			l->pull(150,30);
			t->valveSelect();
			l->pull(150,30);

			cout<<"put tubes 4-8 in 1% SDS. Press ENTER when done."<<endl;
			getString();
			s->valveSelect();
			l->pull(150,30);
			d->valveSelect();
			l->pull(150,30);
			p->valveSelect();
			l->pull(150,30);
			n->valveSelect();
			l->pull(150,30);
			t->valveSelect();
			l->pull(150,30);

			cout<<"put tubes 4-8 in water. Press ENTER when done."<<endl;
			getString();
			s->valveSelect();
			l->pull(1000,100);
			d->valveSelect();
			l->pull(1000,100);
			p->valveSelect();
			l->pull(1000,100);
			n->valveSelect();
			l->pull(1000,100);
			t->valveSelect();
			l->pull(1000,100);
			return;
			break;

		case '5': //prime solutions on syringe
			h->pull(h->injectTubingVolumePull,w->sd.uLps);
			b->pull(b->injectTubingVolumePull,w->sd.uLps);
			e->pull(e->injectTubingVolumePull,w->sd.uLps);
			return;
			break;

		case '6':
			//empty syringe
			e->waste();
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<3; i++)
			{
				e->pull(e->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			e->waste();
			return;
			break;

		case '7':
			//empties syringe
			h->waste();
			//wash syringe 1x with dh2o
			h->pull(h->s->volume,100);
			h->waste();
			//wash syringe and loop line 3x with dh2o
			for (int i=0; i<3; i++)
			{
				h->pull(h->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			h->waste();
			return;
			break;

		case '8':
			//prime NHS-PEG 
			n->valveSelect();
			l->pull(n->sd.inletTubingVol,n->sd.uLps);
			w->valveSelect();
			l->push(n->sd.inletTubingVol,w->sd.uLps);
			//load NHS-PEG
			n->valveSelect();
			l->pull(f->sd.inletTubingVol+15,n->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			cout<<"NHS-PEG blocking rxn start."<<endl;
			Timer::wait(1*60*1000);//react for 1 hour
			cout<<"NHS-PEG blocking rxn end."<<endl;
			//wash with DMF, then with ethanol
			d->valveSelect();
			l->pull(f->sd.inletTubingVol,d->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol,f->sd.uLps);
			//empty syringe and line of DMF, fill with ethanol
			e->waste();
			//wash syringe 1x with ethanol
			e->pull(e->s->volume,100);
			e->waste();
			//wash syringe and loop line 3x with ethanol
			for (int i=0; i<1; i++)
			{
				e->pull(e->s->volume,100);
				w->valveSelect(); 
				l->push(l->s->volume,w->sd.uLps);
			}
			//refill syringe with ethanol and wash device channels 2x
			e->pull(f->sd.inletTubingVol*2,100);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);
			//run air through channels to dry
			a->valveSelect();
			l->pull(f->sd.inletTubingVol*2+15,a->sd.uLps);
			f->valveSelect();
			l->push(f->sd.inletTubingVol*2,f->sd.uLps);

			return;
			break;
		case 'e':
			return;
			break;
		}
		
	}

}
*/
void ProtocolWalsh::ivTrans(){
	char c;
	Solution* r=scan.fs.getSolution("reagent");
	Solution* f=scan.fs.getSolution("column");
	Solution* w=scan.fs.getSolution("waste");
	Solution* a=scan.fs.getSolution("air");
	FlowChannel* l=scan.fs.getChannel("loop");
	FlowChannel* h=scan.fs.getChannel("water");

	double r_vol=93.5, f_bvol=119.9, f_avol=64.3;
	cout<<"Select process"<<endl;
	while(true){
		cout<<"1: Wash system"<<endl;
		cout<<"2: Wash fibers and load RNA"<<endl;
		cout<<"p: prime solutions"<<endl;
		cout<<"3: wash fibers"<<endl;
		cout<<"4: Collect Sample"<<endl;
		cout<<"e:Exit"<<endl;
		c=::getChar();
		switch(c){
		case '1':
			//weak base wash
			cout<<"place tubes in 0.1 NaOH solution"<<endl;
			getString();
			r->valveSelect();
			l->waste();
			l->select();
			l->pull(1000,100,5000);
			f->valveSelect();
			l->pull(1500,100,5000);
			l->waste();
			h->select();
			h->pull(1500,100,5000);
			h->waste();
			//detergent wash
			cout<<"place tubes in 0.1% SDS solution"<<endl;
			getString();
			r->valveSelect();
			l->waste();
			l->select();
			l->pull(2500,100,5000);
			f->valveSelect();
			l->pull(2500,100,5000);
			l->waste();
			h->select();
			h->pull(2500,100,5000);
			h->waste();

			//dH2O rinse
			cout<<"place tubes in dH2O"<<endl;
			getString();
			r->valveSelect();
			l->waste();
			l->select();
			l->pull(5000,100,5000);
			f->valveSelect();
			l->pull(5000,100,5000);
			l->waste();
			h->select();
			h->pull(5000,100,5000);
			h->waste();

			break;
		case '2':
			cout<<"Wash fibers using 2xSSC w/ tri + RNase Secure.\n Place reagent line in soln and press enter."<<endl;
			getString();
			dispense(r,1000,l,f,h);

			cout<<"Load RNA soln.\n Place reagent line in soln and press enter."<<endl;
			getString();

			r->valveSelect();
			l->pull(r_vol,20,5000);

			a->valveSelect();
			l->pull(5,5);

			r->valveSelect();
			l->pull(150,20,5000);
			f->valveSelect();
			l->push(250,20,5000);
			

			break;
		case 'p': //prime time
			h->select();
			h->pull(1000,100);
			l->select();
			w->valveSelect();
			l->push(1000,100);

			r->valveSelect();
			l->pull(r_vol+20,20);
			f->valveSelect();
			l->pull(f_bvol+20,20);

			l->waste();
			h->select();
			h->pull(2500,100,5000);
			l->select();
			w->valveSelect();
			l->push(2500,100,5000);

		case '3':
			//prime
			r->valveSelect();
			l->pull(r_vol+20,40);
			w->valveSelect();
			l->push(r_vol+20,40);
			h->select();
			h->pull(500,100);
			l->select();
			l->push(500,100);
			//wash
			r->valveSelect();
			l->pull(400,40);
			f->valveSelect();
			l->push(350,40);
			r->valveSelect();
			l->pull(400,40);
			f->valveSelect();
			l->push(400,40);
			r->valveSelect();
			l->pull(400,40);
			f->valveSelect();
			l->push(400,40);

			break;
		case '4':
			//collect
			h->select();
			h->pull(300,100);
			f->valveSelect();
			l->push(f_avol,20);
			cout<<"Collect sample"<<endl;
			getString();
			l->push(120,20);
		case 'e':
			return;
			break;
		}
		
	}

}

void ProtocolWalsh::pumpKinetics(){
	double totalTime;
	double period=0;
	int numScans, chan;
	bool wait=false;
	string folder,selection;
	stringstream ss;
	cout<<"enter channel number"<<endl;
	chan=getInt();
	cout<<"enter total time in seconds for this scan"<<endl;
	totalTime=getDouble();
	cout<<"start imaging before solution enters channel (y or n)"<<endl;
	char in=::getChar();

	while (in!='y' && in!='n'){
		in=::getChar();
	}
		
		
		if (in=='y')
			wait=false;	
		if (in=='n')
			wait=true;
	FlowChannel* f=scan.fs.getChannel("ch"+toString(chan));
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	::DefiniteFocus df;
	ScanFocus* sf;
	sf=NULL;
	SpotSeriesScan sss(scan.FOVChannels.front().front(),"Scan",totalTime,period,sf);
	sss.runScan(1,false);
	//f->s->waste();
	//f->s->sendCommand(f->s->moveValve(f->syringePos));
	f->select();
	cout<<"press (c) to continue or (b) to go back"<<endl;
	cin>>selection;
	cin.clear();
	std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');

	if (selection!="c") {
		sss.abort();
		
		return;
	}
	cont.df.getDefiniteFocus(protocolIndex,true);
	f->s->push(f->syringePos,25,40,wait,0);
	sss.sendStartEvent();
	
}


void ProtocolWalsh::incorpKinetics(){
	string folder;
	double times;
	bool df=true;
	int chan;
	int FOVNum=0;
	int i=0;
	double delay;
	Solution* s=scan.fs.getSolution("stuff");
	cout<<"enter channel number"<<endl;
	chan=getInt();
	cout<<"definite focus before every picture? (y or n)"<<endl;
	char v=getChar();
		if (v!='y')
			df=false;
	FlowChannel* f=scan.fs.getChannel("ch"+toString(chan));
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	int tnum=0;
	DefiniteFocusScan dfs(scan.FOVChannels.at(0),1,scan.FOVscan,scan.direction,string("T")+toString(tnum,3));
				
	string c;
		cout<<"press (c) to continue or (b) to go back"<<endl;
		cin>>c;
		cin.clear();
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
			if (c!="c")
			return;
			cont.te.setFixedTemp(25);
			cont.te.wait();
			if(df=false){
				cont.df.getDefiniteFocus(protocolIndex,true);
				cont.df.wait();
			}
			cont.setWorkingDir(folder);
			//AcquisitionChannel::takeMultiplePics(scan.FOVChannels.at(FOVNum),string("FOV")+toString(FOVNum,2),"");
			ProtocolEric::imageSingleFOV(0,scan);
			Timer swatch(true);
			f->select();
			f->pull(20,s->sd.uLps,0);
			
			
			for(i;i<11;i++){
				if(i<5)
					if(df=true)
						delay=0;
					else
						delay=5000;
				else
					delay=20000;
				//cont.setWorkingDir(folder+"\\"+toString(i+1));
				//FOVNum=FOVNum+1;
				//AcquisitionChannel::takeMultiplePics(scan.FOVChannels.at(FOVNum),string("FOV")+toString(FOVNum,2),"");
				dfs.runScan();
				times=swatch.getTime();
				logFile.write(toString(times));
				Timer::wait(delay);

			}
			
		
}


struct PumpPull{
	static FlowChannel* f;
	static Solution* s;
static void pumpPullFunc(int i){
	if (i==5){
		//f->s->pull(f->syringePos,20,s->sd.uLps,false,0);
		/*
		
		*/
		f->s->pmp->executeCommand(f->s->devNum);
	}
	if (i%10==0){
		cont.df.getDefiniteFocus(protocolIndex,true);
		cont.df.wait();
	}
}
};

FlowChannel* PumpPull::f=NULL;
Solution* PumpPull::s=NULL;



void ProtocolWalsh::incorpKinetics2(){
/*	string folder;
	bool df=true;
	int chan;
	int FOVNum;
	int i=0;
	double delay;
	Solution* s=scan.fs.getSolution("stuff");
	cout<<"enter channel number"<<endl;
	chan=getInt();
	cout<<"new FOV for each time point? (y or n)"<<endl;
	char v=getChar();
		if (v!='y')
			df=false;
	FlowChannel* f=scan.fs.getChannel("ch"+toString(chan));
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	cont.setWorkingDir(folder);
	stringstream ss;
	string line;
	vector<double> times;
	cout<<"enter number of images and period (separated by commas) for second set of images"<<endl;
	getline(cin,line);
	cont.setWorkingDir(folder);
	ss<<line;
	getline(ss,line,',');
	times.push_back(0);
	times.push_back(0);
	int numImages=toInt(line);
	getline(ss,line,',');
	if (ss.fail()) {
		logFile.write("Invalid input",true);
		return;
	}
	double period=toDouble(line);
	for(int i=1;i<=numImages;i++){
		times.push_back(i*period);
	}
	cout<<"enter number of images and period (separated by commas) for third set of images"<<endl;
	getline(cin,line);
	ss=stringstream();
	ss<<line;
	getline(ss,line,',');
	int numImages2=toInt(line);
	getline(ss,line,',');
	if (ss.fail()) {
		logFile.write("Invalid input",true);
		return;
	}
	double period2=toDouble(line);
	for(int i=1;i<=numImages2;i++){
		times.push_back(numImages*period+i*period2);
	}
	f->s->waste();
	f->select();
	f->s->sendCommand(f->s->moveValve(f->syringePos));
	double time;
	f->s->pmp->sendCommandNoExecute(f->s->_pull(f->syringePos,20,s->sd.uLps,&time,0),f->s->devNum);

	::DefiniteFocus df2;
	::DefiniteFocus dfDummy(false);
	ScanFocus* sf;
	if (scan.coarse.range==0)
		sf=&dfDummy;
	else
		sf=&df2;
	PumpPull::f=f;
	PumpPull::s=s;
	string c;
	SpotSeriesScan sss(scan.FOVChannels.front(),"Scan",times,sf,df, scan.direction,scan.FOVscan,PumpPull::pumpPullFunc);
	sss.enableStartEvent();

		cout<<"press (c) to continue or (b) to go back"<<endl;
		cin>>c;
		cin.clear();
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
			if (c!="c")
			return;
			cont.df.getDefiniteFocus(protocolIndex,true);
			cont.df.wait();
			sss.runScan(1,false);
			sss.sendStartEvent();
			sss.wait();*/
}


struct PumpPush{
	static FlowChannel* f;
	static Solution* s;
static void pumpPushFunc(int i){
	if (i==1){
		f->push(10,s->sd.uLps,0);
	}
}
};

FlowChannel* PumpPush::f=NULL;
Solution* PumpPush::s=NULL;
void ProtocolWalsh::incorpKinetics3(){
	string folder;
	bool df=true;
	int chan;
	int FOVNum;
	int i=0;
	double delay;
	Solution* s=scan.fs.getSolution("stuff");
	cout<<"enter channel number"<<endl;
	chan=getInt();
	cout<<"definite focus before every picture? (y or n)"<<endl;
	char v=getChar();
		if (v!='y')
			df=false;
	FlowChannel* f=scan.fs.getChannel("ch"+toString(chan));
	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	stringstream ss;
	string line;
	vector<double> times;
	cout<<"enter number of images and period (separated by commas) for second set of images"<<endl;
	getline(cin,line);
	ss<<line;
	getline(ss,line,',');
	times.push_back(0);
	times.push_back(0);
	int numImages=toInt(line);
	getline(ss,line,',');
	if (ss.fail()) {
		logFile.write("Invalid input",true);
		return;
	}
	double period=toDouble(line);
	for(int i=1;i<=numImages;i++){
		times.push_back(i*period);
	}
	cout<<"enter number of images and period (separated by commas) for third set of images"<<endl;
	getline(cin,line);
	ss=stringstream();
	ss<<line;
	getline(ss,line,',');
	int numImages2=toInt(line);
	getline(ss,line,',');
	if (ss.fail()) {
		logFile.write("Invalid input",true);
		return;
	}
	double period2=toDouble(line);
	for(int i=1;i<=numImages2;i++){
		times.push_back(numImages*period+i*period2);
	}
	::DefiniteFocus df2;
	PumpPush::f=f;
	PumpPush::s=s;
	string c;
	SpotSeriesScan sss(scan.FOVChannels.front().front(),folder,times,&df2,df, scan.direction,scan.FOVscan,PumpPush::pumpPushFunc);
	sss.enableStartEvent();

		cout<<"press (c) to continue or (b) to go back"<<endl;
		cin>>c;
		cin.clear();
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
			if (c!="c")
			return;
			sss.runScan(1,false);
			f->select();
			sss.sendStartEvent();
			sss.wait();
}

void ProtocolWalsh::hybRxn(double minTemp,double maxTemp){
	char c;
	bool next=false;
	//cont.setWorkingDir("te mod test");
	double totalTime=25*60;
	Timer swatch(true);

	//set TE mod controller to computer controlled
	cont.te.displayControlOff();
	//if oil objective, move away from surface
	if (cont.currentChannel().chan && cont.currentChannel().m->obj.isOil)
	cont.focus->move(0);
	cont.te.displayControlOff();
	//raise temp to maxTemp
		cont.te.setDerGain(0);
	cont.te.setIntegGain(.2);
	cont.te.setPropBand(75);
	cout<<"Heating to "+toString(maxTemp)+" degC"<<endl;
	cont.te.periodicRecording(1,false);
	cont.te.setFixedTemp(maxTemp);
	//	while(cont.te.getTemp()<maxTemp){
	//		t.wait(1000);
		//	tempsHyb.write(toString(cont.te.getTemp())+","+toString(swatch.getTime())+",",true);
	//	}
		cout<<"Holding at "+toString(maxTemp)+" degC for 1 minutes"<<endl;
	//	t.startTimer();
	//	while(t.getTime()<5*60*1000){
		Timer::wait(1*60*1000);
	//	}
		//lower temp @ 0.1 degC/s
		minTemp=20;
		double time=(maxTemp-minTemp)/0.1;
		cont.te.setPropBand(500);
		cont.te.setIntegGain(0.01);
		cont.te.setDerGain(0.1);
		cout<<"Decreasing temp to "+toString(minTemp)+" degC at 0.1 degC/sec"<<endl;
		//cont.te.setFixedTemp(minTemp);
		cont.te.linearTempRamp(minTemp,time/60.0,true);
		cont.te.setDerGain(0);
		cont.te.setFixedTemp(minTemp);
		//cont.te.powerOff();
				
}
void ProtocolWalsh::dilutionEffect(){
	try{
	Solution* g=scan.fs.getSolution("fluorescein");
	Solution* w=scan.fs.getSolution("wash");
	Solution* a=scan.fs.getSolution("air");
	Solution* p;
	FlowChannel* f=scan.fs.getChannel("tube");
	double vol;
	double Q;
	char c;
	while(true){
		cout<<"1: wash tube"<<endl;
		cout<<"2: load fluorescein"<<endl;
		cout<<"3: load air gap"<<endl;
		cout<<"4: move fluorescein"<<endl;
		cout<<"5: pull variable volume and flow rate"<<endl;
		cout<<"6: push variable volume and flow rate"<<endl;
		cout<<"e:exit"<<endl;
		c=::getChar();

		switch(c){
		case '1':
			w->valveSelect();
			f->select();
			f->pull(1000,w->sd.uLps);
			f->wait();
			break;
		case '2':
			g->valveSelect();
			f->select();
			f->pull(20,20);
			f->wait();
			break;
		case '3':
			a->valveSelect();
			f->select();
			f->pull(20,10);
			f->wait();
			break;
		case '4':
			w->valveSelect();
			f->select();
			f->pull(300,30);
			f->wait();
			break;
		case '5':
			p=scan.fs.selectSolution();
			p->valveSelect();
			cout<<"enter volume to pull"<<endl;
			vol=getDouble();
			cout<<"enter flow rate in uLps"<<endl;
			Q=getDouble();
			f->select();
			f->pull(vol,Q);
			f->wait();
			break;
		case '6':
			p=scan.fs.selectSolution();
			p->valveSelect();
			cout<<"enter volume to push"<<endl;
			vol=getDouble();
			cout<<"enter flow rate in uLps"<<endl;
			Q=getDouble();
			f->select();
			f->push(vol,Q);
			f->wait();
			break;
		case 'e':
			return;
			break;
		}
	}
	}catch(std::ios_base::failure e)
			    {
					logFile.write("Dilutoin Test Exception: "+string(e.what()),true);
				}
}

void ProtocolWalsh::cleavePrep(){
	FlowChannel* w=scan.fs.getChannel("dgH2O");
	FlowChannel* h=scan.fs.getChannel("Ph");
	FlowChannel* d=scan.fs.getChannel("Pd");
	FlowChannel* r=scan.fs.getChannel("Argon");
	FlowChannel* m=scan.fs.getChannel("mix");
	char c;
	
	double volPd, volPh, syringeVol, cleaveVol;
	long massPd, massPh, cleavePdVol, cleavePhVol;

	while(true){
		cout<<"Please select process:"<<endl;
		cout<<"1: Create Pd solution"<<endl;
		cout<<"2: Create Ph solution"<<endl;
		cout<<"3: Prime Pd and Ph"<<endl;
		cout<<"4: Create cleavage mix"<<endl;
		cout<<"4: Discard cleavage mix"<<endl;
		cout<<"5: Wash syringe"<<endl;
		c=::getChar();
		switch(c){
		case '1':
			//determine volume of water needed for [] concentration of Pd
			cout<<"Enter mass of Pd in miligrams./n"<<endl;
			massPd=getDouble();
			volPd=massPd*(0.001)/(0.968); //mass of Pd divided by desired concentration in grams/microliter [3.29*10^-3 M *294.21 g/mol]
			cout<<"Loading "+toString(volPd)+" ul of water to Pd"<<endl;
			syringeVol=w->getSyringeVolume(w); 
			w->waste();
			w->select();
			while(volPd>0){
				if(volPd>syringeVol){
					w->pull(syringeVol,300,4000);
					d->select();
					d->push(syringeVol,300,4000);
				}else{
					w->pull(volPd,300,4000);
					d->select();
					d->push(volPd,300,4000);
				}
				volPd=volPd-syringeVol;
			}

		case '2':
			//deterimine volume of water needed for [] concentration of Ph
			cout<<"Enter mass of Ph in miligrams./n"<<endl;
			massPh=getDouble();
			volPh=massPh*(0.001)/(10.004); //mass of Ph divided by desired concentration in grams/microliter [17.6*10^-3 M * 568.41 g/mol]
			cout<<"Loading "+toString(volPh)+" ul of water to Ph"<<endl;
			syringeVol=w->getSyringeVolume(w);
			w->waste();
			w->select();
			while(volPh>0){
				if(volPh>syringeVol){
					w->pull(syringeVol,300,4000);
					h->select();
					h->push(syringeVol,300,4000);
				}else{
					w->pull(volPh,300,4000);
					h->select();
					h->push(volPd,300,4000);
				}
				volPd=volPd-syringeVol;
			}
		case '3':
			//prime Pd
			d->select();
			d->pull(200,30);
			d->waste();
			//prime Ph
			h->select();
			h->pull(200,30);
			h->waste();
		case '4':
			//create cleavage mix
			cout<<"enter volume of cleavage mix desired/n"<<endl;
			cleaveVol=getDouble();
			//load Pd
			cleavePdVol=cleaveVol/8;
			while(cleavePdVol>0){
				if(cleavePdVol>syringeVol){
					d->select();
					d->pull(syringeVol,50,4000);
					m->select();
					m->push(syringeVol,50,4000);
				}else{
					d->pull(volPd,300,4000);
					m->select();
					m->push(volPd,300,4000);
				}
				cleavePdVol=cleavePdVol-syringeVol;
			}
			//load Ph
			cleavePhVol=cleaveVol*7/8;
			while(cleavePhVol>0){
				if(cleavePhVol>syringeVol){
					h->select();
					h->pull(syringeVol,50,4000);
					m->select();
					m->push(syringeVol,50,4000);
				}else{
					h->pull(volPd,300,4000);
					m->select();
					m->push(volPd,300,4000);
				}
				cleavePhVol=cleavePhVol-syringeVol;
			}
			//move into vile
			r->select();
			r->pull(syringeVol,500);
			m->select();
			m->push(syringeVol,500);
		}
	}

}

void ProtocolWalsh::dispense(Solution* sol,double uL,FlowChannel* l=scan.fs.getChannel("loop"),Solution* f=scan.fs.getSolution("column"),FlowChannel* h=scan.fs.getChannel("water")){
	//pull solution into injection loop and load into column line
	double vol=uL;
	while (uL>l->injectTubingVolumePull){
			sol->valveSelect();
			l->select();
			l->pull(l->injectTubingVolumePull,sol->sd.uLps);
			f->valveSelect();
			l->push(l->injectTubingVolumePull,sol->sd.uLps);
			uL=uL-l->injectTubingVolumePull;
	}
		if (uL>0){
			sol->valveSelect();
			l->select();
			l->pull(uL,sol->sd.uLps);
			f->valveSelect();
			l->push(uL,sol->sd.uLps);
		}
	//move solution into column
	//vol to move-->
	double moveVol=f->sd.inletTubingVol-(vol-l->cham.getChannelVolume())/2;
	if (moveVol>0){
		int p=l->s->getPosition();
		double v=l->getSyringeVolume(l);
		double currVol=v/24000*p;
		if (currVol<moveVol){
			h->select();
			h->pull(moveVol-currVol,100);
		}
		l->select();
		l->push(moveVol,f->sd.uLps);
	}
}

void ProtocolWalsh::collect(double uL,FlowChannel* l=scan.fs.getChannel("loop"),Solution* col=scan.fs.getSolution("column"),FlowChannel* h=scan.fs.getChannel("water"),Solution* out=scan.fs.getSolution("out")){
	//move solution from column out of tubing
	//vol to move-->
	double moveVol=out->sd.inletTubingVol+l->cham.getChannelVolume();
	int p=l->s->getPosition();
	double v=l->getSyringeVolume(l);
	double currVol=v/24000*p;
	if (currVol<moveVol){
		h->select();
		h->pull(moveVol-currVol,100);
	}
	l->select();
	l->push(out->sd.inletTubingVol,out->sd.uLps);
	cout<<"Prepare to collect column contents. Press enter to begin collecting"<<endl;
	getString();
	l->push(l->cham.getChannelVolume(),col->sd.uLps);

}

void ProtocolWalsh::chemCleave(){
	try{
	char ch;
	Solution* b=scan.fs.getSolution("Plug");
	Solution* w=scan.fs.getSolution("dgH2O");
	Solution* h=scan.fs.getSolution("Ph");
	Solution* d=scan.fs.getSolution("Pd");
	Solution* v=scan.fs.getSolution("Vacuum");
	FlowChannel* f=scan.fs.getChannel("cleave");
	Solution* r=scan.fs.getSolution("Ar");
	Solution* c=scan.fs.getSolution("cleaveMix");
	Solution* s=scan.fs.getSolution("DNA");
	Solution* p;
	Solution* a=scan.fs.getSolution("Arg");

	double volPd;
	double volPh;
	double volCleav;
	double volDisp;
	double volCleavDisp;

	while(true){
		cout<<"Please select process:"<<endl;
		cout<<"1: Argon purging of water"<<endl;
		cout<<"2: Remove oxygen from Pd and Ph"<<endl;
		cout<<"3: Connect water to common and vac line"<<endl;
		cout<<"4: Load water into Pd"<<endl;
		cout<<"5: Load water into Ph"<<endl;
		cout<<"6: Bubble Ar to mix soln"<<endl;
		cout<<"7: prime Pd and Ph"<<endl;
		cout<<"8: Load Pd and Ph soln into cleavage mix tube"<<endl;
		cout<<"9: prime cleavage mix"<<endl;
		cout<<"f: Load cleavage mix into line"<<endl;
		cout<<"e: Exit"<<endl;
		ch=::getChar();
		switch(ch){
		case '1':
			//bubble argon through H2O to degas
			cout<<"connect Ar to valve common, insert vac to H2O\nPress enter when ready"<<endl;
			getString();
			w->Solution::valveSelect();
			cout<<"Purging..."<<endl;
			Timer::wait(15*60*1000); //wait 15 min
			b->Solution::valveSelect(); //move valve to plug next to H2O to stop Ar flow
			break;
		case '2':
			cout<<"close manual valve on degassed water. remove Ar from common and attach vac\npress enter when ready"<<endl;
			getString();
			h->Solution::valveSelect();
			cout<<"Vacuuming..."<<endl;
			Timer::wait(2*60*1000);
			d->Solution::valveSelect();
			cout<<"Vacuuming..."<<endl;
			Timer::wait(2*60*1000);
			b->Solution::valveSelect();
			cout<<"connect Argon\nPress enter when done"<<endl;
			getString();
			h->Solution::valveSelect();
			Timer::wait(3000);
			d->Solution::valveSelect();
			Timer::wait(3000);
			b->Solution::valveSelect();
			break;
		case '3':
			cout<<"connect degassed water to common, leave manual switch closed\n connect vac to port 8\npress enter when done"<<endl;
			getString();
			v->Solution::valveSelect();
			Timer::wait(5*1000);
			cout<<"connect Ar to port 9, press enter when done"<<endl;
			getString();
			a->Solution::valveSelect();
			Timer::wait(2000);
			b->valveSelect();
			break;
		case '4':
			cout<<"connect vac directly to Pd tube\npress enter when ready to load solution into Pd"<<endl;
			getString();
			d->Solution::valveSelect();
			cout<<"close manual valve to water and press enter when done  to switch valve to plug\n"<<endl;
			getString();
			b->Solution::valveSelect();
			break;
		case '5':
			cout<<"connect vac directly to Ph tube\npress enter when ready to load solution into Ph"<<endl;
			getString();
			h->Solution::valveSelect();
			cout<<"close manual valve to water and press enter when done to switch valve to plug\n"<<endl;
			getString();
			b->Solution::valveSelect();
			break;
		case '6':
			cout<<"bubble Ar through to mix soln\nConnect Ar to common\nPress enter when ready"<<endl;
			getString();
			p=scan.fs.selectSolution();
			p->Solution::valveSelect();
			cout<<"Press enter when done"<<endl;
			getString();
			b->Solution::valveSelect();
			break;
		case '7': //prime inlet tubings, and prime inject tubing line with water
			cout<<"priming Pd and Ph solutoins...\n"<<endl;
			d->valveSelect();
			f->pull(d->sd.inletTubingVol,d->sd.uLps);
			f->wait();
			Timer::wait(1*1000);
			h->valveSelect();
			f->pull(h->sd.inletTubingVol,h->sd.uLps);
			f->wait();
			Timer::wait(1*1000);
			w->valveSelect();
			f->pull(f->injectTubingVolumePull,w->sd.uLps);
			f->waste();
			break;
		case '8': //load Pd and Ph soln into line
			cout<<"enter volume of Pd solution needed"<<endl;
			volPd=getDouble();
			cout<<"enter volume of Ph solution needed"<<endl;
			volPh=getDouble();
			//loading argon gap
			r->valveSelect();
			f->pull(20,r->sd.uLps);
			f->wait();
			Timer::wait(2*1000);
			//loading reagents
			d->valveSelect();
			f->pull(volPd,d->sd.uLps);
			f->wait();
			Timer::wait(2*1000);
			h->valveSelect();
			f->pull(volPh,h->sd.uLps);
			f->wait();
			Timer::wait(2*1000);
			//dispense solution into cleavage mix tube
			c->valveSelect();
			f->push(volPd+volPh+10,c->sd.uLps);
			f->wait();
			//loading argon
			r->valveSelect();
			f->pull(2*(c->sd.inletTubingVol),r->sd.uLps);
			f->wait();
			Timer::wait(2*1000);
			//dispense solution into cleavage mix tube
			c->valveSelect();
			f->push(2*(c->sd.inletTubingVol),c->sd.uLps);
			f->wait();
			break;
		case '9': //prime inlet tubings, and prime inject tubing line with water
			cout<<"priming cleavage mix...\n"<<endl;
			c->valveSelect();
			f->pull(c->sd.inletTubingVol,c->sd.uLps);
			f->wait();
			Timer::wait(1*1000);
			w->valveSelect();
			f->pull(f->injectTubingVolumePull,w->sd.uLps);
			f->waste();
			break;
		case 'f':
			cout<<"enter volume of cleavage solution to load into sample\n"<<endl;
			volCleav=getDouble();

			//loading argon gap
			r->valveSelect();
			f->pull(20,r->sd.uLps);
			f->wait();
			Timer::wait(2*1000);
			//load cleavage mix
			c->valveSelect();
			f->pull(volCleav,c->sd.uLps);
			f->wait();
			Timer::wait(2*1000);
			//dispense solution into DNA sample tube
			s->valveSelect();
			f->push(volCleav+10,s->sd.uLps);
			f->wait();
			//loading argon
			r->valveSelect();
			f->pull(2*(s->sd.inletTubingVol),r->sd.uLps);
			f->wait();
			Timer::wait(2*1000);
			//dispense solution into cleavage mix tube
			s->valveSelect();
			f->push(2*(c->sd.inletTubingVol),s->sd.uLps);
			f->wait();
			break;

	case 'L': //load Ar gap
			r->valveSelect();
			f->pull(20,r->sd.uLps);
			f->wait();
			Timer::wait(3*1000);
			b->valveSelect();
			break;

		case 'e':
			return;
			break;

		}
	}
	}catch(std::ios_base::failure e)
			    {
					logFile.write("Cleavage Exception: "+string(e.what()),true);
				}
}

void ProtocolWalsh::cycleSeq(){
	string folder;
	string c;
	Solution* w=scan.fs.getSolution("wash");
	Solution* n=scan.fs.getReagent("Incorp_reagent");
	Solution*l=scan.fs.getReagent("Cleave_reagent");
	FlowChannel* f=scan.fs.selectChannel();
	scan.FOVscan=0;
	cout<<"Enter number of cycles to perform"<<endl;
	int numCycles=getInt();
	cout<<"Enter reaction temperature in Celcius"<<endl;
	double rxnTemp=getDouble();
	cont.te.setFixedTemp(rxnTemp);
	cout<<"Enter reaction time in minutes"<<endl;
	double rxnTime=getDouble();

	cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
	std::getline(std::cin,folder);
	int cycleNum=0;
	
				
		cout<<"press (c) to continue or (b) to go back"<<endl;
		cin>>c;
		cin.clear();
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
			if (c!="c")
			return;

	for(int i=0;i<numCycles;i++){
		rxnLoad(n,f,rxnTime); //incorp rxn
		w->sd.s->pull(w->sd.syringePort,500,w->sd.uLps);
		f->push(500,w->sd.uLps);
		w->sd.s->pull(w->sd.syringePort,500,w->sd.uLps);
		f->push(500,w->sd.uLps);
		{
		cont.setWorkingDir(folder  +string("\\incorp"));
		DefiniteFocusScan dfs(scan.FOVChannels.at(0),1,scan.FOVscan,scan.direction,string("Cycle")+toString(cycleNum,3));
		dfs.runScan();
		}
		
		rxnLoad(l,f,rxnTime); //cleave rxn
		w->sd.s->pull(w->sd.syringePort,500,w->sd.uLps);
		f->push(500,w->sd.uLps);
		w->sd.s->pull(w->sd.syringePort,500,w->sd.uLps);
		f->push(500,w->sd.uLps);
		{
		cont.setWorkingDir(folder +string("\\cleave"));
		DefiniteFocusScan dfs(scan.FOVChannels.at(0),1,scan.FOVscan,scan.direction,string("Cycle")+toString(cycleNum,3));
		dfs.runScan();
		}
		}

	
}

void ProtocolWalsh::reagLoad(Solution* r,vector<FlowChannel*> f){
	double vol;

	Solution* w=scan.fs.getSolution("Waste");
	r->sd.isInletPrimed=true;
	cout<<"Is inlet primed (y/n)"<<endl;
	char v=getChar();
	//prime inlet tubing if nec.
	if (v!='y')
		r->sd.isInletPrimed=false;
	char l='q';
	while(l!='i' && l!='b' && l!='e'){
		cout<<"Load reagent into channel or stop before entering the channel? Type 'i' for into or 'b' for before the channel, or 'e' to exit."<<endl;
		l=getChar();
	}
	if(l=='e')
		return;
	r->sd.s->waste();
	if(r->sd.isInletPrimed==false){
		r->sd.wash->sd.s->pull(r->sd.wash->sd.syringePort,100,r->sd.wash->sd.uLps);
		r->valveSelect();
		r->sd.s->pull(r->sd.syringePort,r->sd.inletTubingVol,r->sd.uLps);
		w->valveSelect();
		r->sd.s->push(r->sd.syringePort,r->sd.inletTubingVol+100,r->sd.wash->sd.uLps);
	}
	for(vector<FlowChannel*>::iterator i=f.begin();i!=f.end();i++){
		//pull in wash buffer to prepare for moving to chamber
		r->sd.wash->sd.s->pull(r->sd.wash->sd.syringePort,(*i)->injectTubingVolumePush,r->sd.wash->sd.uLps);
		//load air gap
		r->sd.air->valveSelect();
		r->sd.air->sd.s->pull(r->sd.air->sd.syringePort,r->sd.air->sd.loadFactor*(*i)->cham.getChannelVolume(),r->sd.air->sd.uLps);
		//pull reagent into inj loop
		r->valveSelect();
		r->sd.s->pull(r->sd.syringePort,r->sd.loadFactor*(*i)->cham.getChannelVolume(),r->sd.uLps);
		//load air gap
		r->sd.air->valveSelect();
		r->sd.air->sd.s->pull(r->sd.air->sd.syringePort,r->sd.air->sd.loadFactor*(*i)->cham.getChannelVolume(),r->sd.air->sd.uLps);
		//push into channel
		(*i)->select();
		if(l=='b')
			vol=(*i)->injectTubingVolumePush;
		else
			vol=(*i)->injectTubingVolumePush + r->sd.air->sd.loadFactor*(*i)->cham.getChannelVolume() + r->sd.loadFactor*(*i)->cham.getChannelVolume()/2;
		(*i)->push(vol,r->sd.uLps);
	}
}

void ProtocolWalsh::rxnLoad(Solution* r,FlowChannel* f,double time){
	Solution* b=NULL;
	double vol_wash, vol;
	if(r->sd.name=="Incorp_reagent" || r->sd.name=="Cleave_reagent"){
		if(r->sd.name=="Incorp_reagent")
			b=scan.fs.getReagent("Incorp_buffer");
		else
			b=scan.fs.getReagent("Cleave_buffer");
		//pull reagent prep-wash into loop
		b->valveSelect();
		b->sd.s->pull(b->sd.syringePort,(b->sd.loadFactor+r->sd.air->sd.loadFactor)*f->cham.getChannelVolume(),b->sd.uLps);

		//air gap
		b->sd.air->valveSelect();
		b->sd.air->sd.s->pull(b->sd.air->sd.syringePort,b->sd.air->sd.loadFactor*f->cham.getChannelVolume()*2,b->sd.air->sd.uLps);
		b->valveSelect();
		b->sd.s->push(b->sd.syringePort,b->sd.air->sd.loadFactor*f->cham.getChannelVolume(),b->sd.air->sd.uLps);

		//load pre-wash into flow channel line
		vol_wash=(b->sd.air->sd.loadFactor*2+b->sd.loadFactor)*f->cham.getChannelVolume();
		f->push(vol_wash,b->sd.uLps);
	}
	//reagent
	//pull reagent into loop
	r->valveSelect();
	r->sd.s->pull(r->sd.syringePort,(r->sd.loadFactor+r->sd.air->sd.loadFactor)*f->cham.getChannelVolume(),r->sd.uLps);

	//air gap
	r->sd.air->valveSelect();
	r->sd.air->sd.s->pull(r->sd.air->sd.syringePort,r->sd.air->sd.loadFactor*f->cham.getChannelVolume()*2,r->sd.air->sd.uLps);
	r->valveSelect();
	r->sd.s->push(r->sd.syringePort,r->sd.air->sd.loadFactor*f->cham.getChannelVolume(),r->sd.air->sd.uLps);
	
	//load reagent into flow channel line
	double vol_reag=(r->sd.air->sd.loadFactor*2+r->sd.loadFactor)*f->cham.getChannelVolume();
	f->push(vol_reag,r->sd.uLps);

	if(b!=NULL){
		//move front of pre-wash into channel
		vol_wash=(b->sd.air->sd.loadFactor*2+b->sd.loadFactor)*f->cham.getChannelVolume();
		vol=f->injectTubingVolumePush - vol_wash - vol_reag;
		double move1=vol + vol_wash/2 + f->cham.getChannelHeight()/2;
		r->sd.wash->sd.s->pull(r->sd.wash->sd.syringePort,move1,r->sd.wash->sd.uLps);
		f->push(move1,r->sd.uLps);
		Timer::wait(30*1000);
		vol=0;
	}
	else{
		vol=f->injectTubingVolumePush - vol_reag;
		vol_wash=f->cham.getChannelVolume();
	}
	double move2= vol + vol_wash/2 + vol_reag/2;
	r->sd.wash->sd.s->pull(r->sd.wash->sd.syringePort,move2+300,r->sd.wash->sd.uLps);
	f->push(move2,r->sd.uLps);
	Timer::wait(time*1000);
	f->push(300,r->sd.wash->sd.uLps);

}

void ProtocolWalsh::pumpProtocols(){
	char c; char k; int chan;

	string t;//temp string for input output
	string t2;
	string t3;
	string filename;

	while(true){
		try{
			cout<<"Please select a task"<<endl;


			cout<<"0: "<<endl;
			cout<<"1: "<<endl;
			cout<<"2: Dispense solution"<<endl;
			cout<<"3: "<<endl;
			cout<<"w: Wash"<<endl;
			cout<<"6: Determine inject tubing volume"<<endl;
			cout<<"7: Determine inlet tubing volume"<<endl;
			cout<<"9: Waste"<<endl;
			cout<<"l: Load Reagents"<<endl;
			cout<<"f: fluidics control"<<endl;
			cout<<"p: select solution and pull"<<endl;
			cout<<"q: select flow channel or waste and push"<<endl;
			cout<<"o: select reagent and pull"<<endl;
			cout<<"n: prime reagent(s)"<<endl;
			cout<<"m: prime channel(s)"<<endl;
			cout<<"d: degas column"<<endl;
			cout<<"g: Load reagent"<<endl;
			cout<<"j: wash reagent inlet lines and channel outlets."<<endl;
			cout<<"e: Exit Pump Control"<<endl;
			c=getChar();
			switch(c){
			
			case '0':{//wash inlet tubing by pushing out
				Solution* w=scan.fs.getSolution("Waste");
				cout<<"Select reagent(s) tubing to wash"<<endl;
				vector<Solution*> l=scan.fs.selectReagents();
				cout<<"Select solution with which to wash inlet tubing"<<endl;
				Solution* s=scan.fs.selectSolution();
				//wash inj loop
				s->sd.s->waste();
				s->sd.s->pull(s->sd.syringePort,s->getInjectTubingVolume()+50,w->sd.uLps);
				w->valveSelect();
				w->sd.s->push(w->sd.syringePort,s->getInjectTubingVolume()+50,w->sd.uLps);
				//wash inlets
				for(vector<Solution*>::iterator i=l.begin();i!=l.end();i++){
					s->sd.s->pull(s->sd.valvePos,500,w->sd.uLps);
					(*i)->valveSelect();
					(*i)->sd.s->push((*i)->sd.syringePort,500,s->sd.uLps);
				}
				break;
			 }
			case'j':{//clean system
				char c='a';
				while(c!='p' && c!='e'){
					cout<<"Remove chamber and place device over waste dish.\nPlace all reagent tubing lines in NaOH.\nPress 'p' to proceed, 'e' to exit"<<endl;
					c=getChar();
				}
				if(c=='e')
					return;
				vector<Solution*> r=scan.fs.reagents;
				vector<FlowChannel*> f=scan.fs.channels;
				//wash reagent inlet lines, injection loop, and syringe
				for(vector<Solution*>::iterator i=r.begin();i!=r.end();i++){
					(*i)->valveSelect();
					(*i)->sd.s->pull((*i)->sd.syringePort,500,100);
				}
				//wash flow channel injection lines
				for(vector<FlowChannel*>::iterator i=f.begin();i!=f.end();i++){
					r.front()->valveSelect();
					r.front()->sd.s->pull(r.front()->sd.syringePort,500,100);
					(*i)->select();
					(*i)->push(500,100);
				}
				c='a';
				while(c!='p' && c!='e'){
					cout<<"Place all reagent tubing lines in H2O.\nPress 'p' to proceed, 'e' to exit"<<endl;
					c=getChar();
				}
				if(c=='e')
					return;
				//wash reagent inlet lines, injection loop, and syringe
				for(vector<Solution*>::iterator i=r.begin();i!=r.end();i++){
					(*i)->valveSelect();
					(*i)->sd.s->pull((*i)->sd.syringePort,500,100);
				}
				//wash flow channel injection lines
				for(vector<FlowChannel*>::iterator i=f.begin();i!=f.end();i++){
					r.front()->valveSelect();
					r.front()->sd.s->pull(r.front()->sd.syringePort,500,100);
					(*i)->select();
					(*i)->push(500,100);
				}
					break;}
			case 'g':{
				Solution* r=scan.fs.selectReagent();
				vector<FlowChannel*> f=scan.fs.selectChannels();
				reagLoad(r,f);
				break;
					 }
			
			case 'd':{//degas syringe
				Solution* s=scan.fs.selectSolution();
				int i=0;
				while(i<3){
					i++;
					s->sd.s->waste();
					s->valveSelect();
					s->sd.s->pull(s->sd.syringePort,s->sd.s->volume,100);
				}
				break;
					 }
			case 'f':{
				scan.fs.FluidicsSetup::fluidicsControl();
				break;}
		
			case 'p':{
					//pull solution on pump into syringe
					Solution* l=scan.fs.selectSolution();
					cout<<"Pulling solution "+string(l->sd.name)+": Enter volume to pull."<<endl;
					double vol=getDouble();
					l->valveSelect();
					l->sd.s->pull(l->sd.syringePort,vol,l->sd.uLps);
					break;
					/*
					Solution* s=scan.fs.selectSolution();
					s->valveSelect();
					FlowChannel* f=scan.fs.selectChannel();
					cout<<"enter volume to pull"<<endl;
					double vol=getDouble();
					f->pull(vol,s->sd.uLps);*/
					}
			
			case 'q':{//pushing- either to reagent or channel
						cout<<"Push into solutoin (s), reagent (r) or channel(c)? Enter 's', 'r' or 'c', 'a' to abort."<<endl;
						char c=getChar();
						/*while((c!='c') || (c!='r') || (c!='a')){
							cout<<"Please enter 'r' or 'c', 'a' to abort."<<endl;
							char c=getChar();
						}*/
						if (c=='s'){
							Solution* s=scan.fs.selectSolution();
							cout<<"Enter volume to push out "+string(s->sd.name)+"."<<endl;
							double vol=getDouble();
							s->valveSelect();
							s->sd.s->push(s->sd.syringePort,vol,s->sd.uLps);
						}
						else if(c=='r'){
							Solution* r=scan.fs.selectReagent();
							cout<<"Enter volume to push out "+string(r->sd.name)+"."<<endl;
							double vol=getDouble();
							r->valveSelect();
							r->sd.s->push(r->sd.syringePort,vol,r->sd.uLps);
						}
						else if(c=='c'){
							FlowChannel* f=scan.fs.selectChannel();
							cout<<"Enter volume to push out "+string(f->name)+"."<<endl;
							double vol=getDouble();
							f->push(vol,10);
						}
						//if(c=='a')
						else
							return;
					 
						/*
						Solution* s=scan.fs.selectSolution();
						s->valveSelect();
						FlowChannel* f=scan.fs.selectChannel();
						f->select();
						cout<<"enter volume to push"<<endl;
						double vol=getDouble();
						f->push(vol,s->sd.uLps);
						*/
						break;
					 }
			case 'o':{//pull reagent
						Solution* r=scan.fs.selectReagent();
						cout<<"Pulling Reagent "+string(r->sd.name)+": Enter volume to pull."<<endl;
						double vol=getDouble();
						r->valveSelect();
						r->sd.s->pull(r->sd.syringePort,vol,r->sd.uLps);
						break;
					 }
			case 'm':{//prime- flow  channels
				Solution* s=scan.fs.selectSolution();
				vector<FlowChannel*> f=scan.fs.selectChannels();
				primeChannels(s,f);
				break;
					 }
			case 'n':{//prime- reagent inlet lines
							bool airGap=true;
							cout<<"Use air gaps?"<<endl;
							char v=getChar();
								if (v!='y')
									airGap=false;
							Solution* w=scan.fs.getSolution("Waste");
							Solution* s=scan.fs.getSolution("wash");
							vector<Solution*> r=scan.fs.selectReagents();
													
							for(vector<Solution*>::iterator i=r.begin();i!=r.end();i++){
								(*i)->sd.s->waste();
								s->valveSelect();
								s->sd.s->pull(s->sd.syringePort,100,s->sd.uLps);
								(*i)->valveSelect();
								(*i)->sd.s->pull((*i)->sd.syringePort,(*i)->sd.inletTubingVol,(*i)->sd.uLps);	
								if(airGap==true){
									(*i)->sd.air->valveSelect();
									(*i)->sd.s->pull((*i)->sd.air->sd.syringePort,(*i)->sd.air->sd.loadFactor*scan.fs.channels.front()->cham.getChannelVolume()*2,(*i)->sd.air->sd.uLps);
									(*i)->valveSelect();
									(*i)->sd.s->push((*i)->sd.syringePort,(*i)->sd.air->sd.loadFactor*scan.fs.channels.front()->cham.getChannelVolume(),(*i)->sd.air->sd.uLps);
									w->valveSelect();
									w->sd.s->push(w->sd.syringePort,(*i)->sd.inletTubingVol+100+(*i)->sd.air->sd.loadFactor*scan.fs.channels.front()->cham.getChannelVolume(),w->sd.uLps);
								}
								else{
								w->valveSelect();
								w->sd.s->push(w->sd.syringePort,(*i)->sd.inletTubingVol+100,w->sd.uLps);
								}
							}
						 }
							break;

			case 'l':{
				cout<<"Reaction reagent loading..."<<endl;
				Solution* r=scan.fs.selectReagent();
				FlowChannel* f=scan.fs.selectChannel();
				cout<<"Enter reaction time in seconds"<<endl;
				double time=getDouble();
				rxnLoad(r,f,time);

				/*
				//loading solution into chamber
				while(true){
					try{	
						cout<<"Please select a task"<<endl;
						cout<<"1: Load 20 uL in front of channel"<<endl;
						cout<<"2: Load 20 uL into channel"<<endl;
						cout<<"e: Exit Submenu"<<endl;
						k=getChar();
						switch(k){

						case '1':{
							FlowChannel* f=scan.fs.selectChannel();
							//load air gap
							f->pull(2,10);
							cout<<"place tube in reagent and press enter"<<endl;
							getString();
							f->pull(20,10);
							cout<<"remove tube from reagent and press enter"<<endl;
							getString();
							f->pull(2,10);
							cout<<"place tubing in solution and press enter"<<endl;
							getString();
							f->pull(43,10);

							break;}

						case '2':{
							FlowChannel* f=scan.fs.selectChannel();
							//load air gap
							f->pull(2,10);
							cout<<"place tube in reagent and press enter"<<endl;
							getString();
							f->pull(20,10);
							cout<<"remove tube from reagent and press enter"<<endl;
							getString();
							f->pull(2,10);
							cout<<"place tubing in solution and press enter"<<endl;
							getString();
							f->pull(63,10);
						
							break;}

						case 'e':
							return;
							break;
						default:
							cout<<c<<" is not a valid option"<<endl<<endl;
						}
					}catch(std::ios_base::failure e)
					{
						logFile.write("ProtocolWalshException: "+string(e.what()),true);
						continue;
					}
				}

				*/
				break;
					 }


			case '2':{
				cout<<"enter volume to dispense"<<endl;
				double uL=getDouble();
				Solution* sol=scan.fs.selectSolution();
				dispense(sol,uL);

				break;}

			case '6':{
				//Determine volume of tubing
				//select channel for determining volume
				FlowChannel* f=scan.fs.selectChannel();
				f->wait();
				//select solution for priming system
				Solution* s=scan.fs.selectSolution();
				s->Solution::valveSelect();
				cout<<"enter volume for initial prime\n";
				double vol=getDouble();
				f->pull(vol,150);
				f->wait();
				Timer::wait(10000);
				//load air gap
				Solution* a=scan.fs.getSolution("Air");
				a->Solution::valveSelect();
				//wait?
				f->pull(20,s->sd.uLps);
				f->wait();
				Timer::wait(3000);
				s->Solution::valveSelect();
				double rec=0;
				//move air gap specified volume
				while(true){
					cout<<"1: moving airgap\n";
					cout<<"e: exit\n";
					c=getChar();
					switch(c){
					case '1': {
						cout<<"Enter volume to move air gap.\nCurrently at " + toString(rec) + "\n";
						double moveVol=getDouble();
						rec=rec+moveVol;
						if (moveVol > 0 ){
							f->pull(moveVol,s->sd.uLps);
						}else {
							f->push(-moveVol,s->sd.uLps);
						}
						logFile.write("Total move volume is " + toString(rec),true);
							  break;}
					case 'e': {
						return;
						break;}
					default:
						cout<<c<<" is not a valid option"<<endl<<endl;
		
					}
				}
			 }

			case '7':{
				//pull volume larger than inlet tubing volume
				FlowChannel* f=scan.fs.selectChannel();
				f->wait();
				//select solution for priming system
				Solution* s=scan.fs.selectSolution();
				s->Solution::valveSelect();
				cout<<"enter volume for initial prime\n";
				double vol=getDouble();
				f->pull(vol,75);
				f->wait();
				Timer::wait(10000);
				//load air gap
				Solution* a=scan.fs.getSolution("Air"); //clear away inline solution
				a->Solution::valveSelect();
				//wait?
				f->pull(1000,s->sd.uLps);
				f->wait();
				Timer::wait(3000);
				s->Solution::valveSelect();
				cout<<"Remove inlet from water\n"<<endl;
				while(true){
					cout<<"1: moving water plug\n";
					cout<<"e: exit\n";
					c=getChar();
					switch(c){
					case '1': {
						cout<<"Enter volume to position where linear distance an be measured.\n"<<endl;
						double moveVol=getDouble();
						if (moveVol > 0 ){
							f->pull(moveVol,s->sd.uLps);
						}else {
							f->push(-moveVol,s->sd.uLps);
						}
							  break;}
					case 'e': {
						return;
						break;}
					default:
						cout<<c<<" is not a valid option"<<endl<<endl;
		
					}
				}
				break;}

			case '9':{
				FlowChannel* f=scan.fs.selectChannel();
				f->waste();
				break;}

			case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
					}
				}catch(std::ios_base::failure e)
				{
					logFile.write("ProtocolWalshException: "+string(e.what()),true);
					continue;
				}
				catch(exception& e){
					logFile.write(string(e.what()),true);
					continue;
				}
					 }

}

void ProtocolWalsh::primeChannels(Solution* s,vector<FlowChannel*> f){
	Solution* w=scan.fs.getSolution("Waste");
	s->sd.s->waste();
	int x=0;
	for(vector<FlowChannel*>::iterator i=f.begin();i!=f.end();i++){
		x=x+1;
		if(x==1){
			s->sd.s->pull(s->sd.syringePort,(*i)->dvPush.injectTubingVol+50,s->sd.uLps);
			w->valveSelect();
			w->sd.s->push(w->sd.syringePort,(*i)->dvPush.injectTubingVol+50,w->sd.uLps);
		}
		s->sd.s->pull(s->sd.syringePort,(*i)->injectTubingVolumePush+50,s->sd.uLps);
		(*i)->push((*i)->injectTubingVolumePush+50,s->sd.uLps);
	}
}

void ProtocolWalsh::loadHyb(){//load reagents and perform hybridization then wash
	Solution* r=scan.fs.getReagent("Surf_prep_reag");
	vector<FlowChannel*> f=scan.fs.selectChannels();
	reagLoad(r,f);
	hybRxn(20,65);
	for(vector<FlowChannel*>::iterator i=f.begin();i!=f.end();i++)
		r->sd.wash->load(*i);
}

void ProtocolWalsh::cDNAsynth(vector<FlowChannel*> f){//synthesize cDNA, denature template
	Solution* r=scan.fs.getReagent("Surf_prep_reag");
	Solution* t=scan.fs.getSolution("TEbuffer");
	r->sd.wash=scan.fs.getSolution("BstBuffer");
	cont.te.setFixedTemp(10);
	reagLoad(r,f);
	cont.te.setFixedTemp(50);
	Timer::wait(3*60*1000);
	cont.te.setFixedTemp(85,false);
	for(vector<FlowChannel*>::iterator i=f.begin();i!=f.end();i++){
		t->load(*i);
	}
	cont.te.setFixedTemp(85);
	Timer::wait(3*60*1000);
	for(vector<FlowChannel*>::iterator i=f.begin();i!=f.end();i++){
		t->load(*i);
		t->load(*i);
	}
}

void ProtocolWalsh::customScan(){
	/*

	//string workingDir="default";
	if (protocolIndex==-1 || !loadProtocol(WORKINGDIR+"\\customScans\\"+protocolFiles[protocolIndex-1],scan)){
		if (!selectProtocol(scan))
			return;
	}
	int m;
	for(vector<vector<AcquisitionChannel>>::reverse_iterator i=scan.FOVChannels.rbegin();i!=scan.FOVChannels.rend();i++){
		for(vector<AcquisitionChannel>::reverse_iterator j=i->rbegin();j!=i->rend();j++){
			cont.addDefaultAC(*j,m,m);
		}
	}
	cont.setCurrentChannel(scan.FOVChannels.front().front());
	cont.currentFocus=scan.coarse;

	if (currentChamber.maxZ==0 && scan.cham.maxZ!=0){
		logFile.write(string("Chamber configuration has been defined, z focus is ")+::toString(scan.cham.focusZ)+" and specified channel is"+::toString(scan.channelNum),true);
		logFile.write("Do you want to move there (y or n)?",true);
		currentChamber=scan.cham;
		cont.axio.setMaxZ(currentChamber.maxZ);
		string yORn;
			while(true){
				yORn=::getString();
				if (yORn=="y"){
					cont.focus->move(currentChamber.focusZ);
					scan.cham.move(scan.channelNum,0.5,scan.fractionLength);
					break;
				}
				if (yORn=="n")
					break;
			}
	}
	
	*/
	//END Load Protocol//
	char c;
	while(true){
		try{
		cout<<"Please select a task for this custom scan:"<<endl;
		cout<<"1: set working directory"<<endl;
		cout<<"2: cleavage"<<endl;
		/*cout<<"0: changed loaded configuration file (currently "<<protocolFiles[protocolIndex-1]<<")"<<endl;
		cout<<"1: change working directory (currently "<<cont.workingDir<<")"<<endl;*/
		cout<<"e: Exit"<<endl;
		cin>>c;
		/*
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		*/
		switch(c){
			/*
			case '0':
				if (!selectProtocol(scan))
					return;
				for(vector<vector<AcquisitionChannel>>::reverse_iterator i=scan.FOVChannels.rbegin();i!=scan.FOVChannels.rend();i++){
		for(vector<AcquisitionChannel>::reverse_iterator j=i->rbegin();j!=i->rend();j++){
			cont.addDefaultAC(*j,m,m);
		}
	}
	cont.setCurrentChannel(scan.FOVChannels.front().front());
	cont.currentFocus=scan.coarse;
				break;
				//		case '1':
				//			TIRFadjust();
				//			break;
				//		case '2':
				//			intensityAdjust();
				//			break;
			case '1':{
				string folder;
				cout<<"enter working folder (will be placed under "<<WORKINGDIR<<")"<<endl;
				cin>>folder;
				cin.clear();
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				cont.setWorkingDir(folder);
				break;
					 }
					 */
			case '1': {
				string folder;
				cout<<"enter folder to save images to (will be placed under "<<WORKINGDIR<<")"<<endl;
				std::getline(std::cin,folder);
				cont.setWorkingDir(folder);
				break;}

			case '2' : {
				break;}
			case 'e':
				return;
				break;
		}
				}catch(std::ios_base::failure e)
			    {
					logFile.write("Pump Control Menu Exception: "+string(e.what()),true);
					continue;
				}
		
	}
}

#endif
