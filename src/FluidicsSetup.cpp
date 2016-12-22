// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Saturday, April 07, 2012</lastedit>
// ===================================
#include "FluidicsSetup.h"
#include "FlowChannel.h"
#include "Reagent.h"
#include "Controller.h"

#include "Math.h"
using namespace std;

extern Controller cont;
extern Record logFile;
extern customProtocol scan;
void FluidicsSetup::primeAll(string wash,string chan){
	FlowChannel* f=NULL;
	for(vector<Solution*>::iterator i=solutions.begin();i!=solutions.end();i++){
		if ((*i)->sd.name!="PullAir" && (*i)->sd.name!="PullAir2" && (*i)->sd.name!="PushAir" && (*i)->sd.name!="PushAir2")
			f=(*i)->prime(f);
	}
}

FlowChannel* FluidicsSetup::getChannel(string name){
	removeWhite(name);
	for(vector<FlowChannel*>::iterator i=this->channels.begin();i!=channels.end();i++){
		if ((*i)->name==name)
			return *i;
	}
	throw exception(string("Channel "+name+" not found in FluidicsSetup. Check protocol file").c_str());
}

Solution* FluidicsSetup::getSolution(string name){
	removeWhite(name);
	for(vector<Solution*>::iterator i=this->solutions.begin();i!=solutions.end();i++){
		if ((*i)->sd.name==name)
			return *i;
	}
	throw exception(string("Solution "+name+" not found in FluidicsSetup. Check protocol file").c_str());
}

Solution* FluidicsSetup::getReagent(string name){
	removeWhite(name);
	for(vector<Solution*>::iterator i=this->reagents.begin();i!=reagents.end();i++){
		if ((*i)->sd.name==name)
			return *i;
	}
	throw exception(string("Reagent "+name+" not found in FluidicsSetup. Check protocol file").c_str());
}

Solution* FluidicsSetup::selectSolutionReagent(){
	cout<<"Wash or Reagent? (w or r)"<<endl;
	char c=getChar();
	if (c=='w')
		return selectSolution();
	if (c=='r')
		return (Solution*) selectReagent();
	else return selectSolutionReagent();

}
Solution* FluidicsSetup::selectSolution(){
	if (solutions.size()<1){
		throw exception("No solutions to select from");
	}
	if (solutions.size()==1){
		logFile.write("Only one solution available:"+ solutions.front()->sd.name,true);
		return solutions.front();
	}
	cout<<"Please select a Solution"<<endl;
	int ind=1;
	for(vector<Solution*>::const_iterator i=solutions.begin();i!=solutions.end();i++){
		cout<<::toString(ind,2)<<": "<<(*i)->toString()<<endl;
		ind++;
	}
	cout<<" a: abort"<<endl;
	string l=getString();
	if (l=="a")
		throw exception("Solution selection aborted");
	ind=toInt(l);
	if (ind>solutions.size() || ind<1){
		logFile.write("invalid solution selected.",true);
		return selectSolution();
	}
	return solutions.at(ind-1);
}

Solution* FluidicsSetup::selectReagent(){
	if (reagents.size()<1){
		throw exception("No reagents to select from");
	}
	if (reagents.size()==1){
		logFile.write("Only one reagent available:"+ reagents.front()->sd.name,true);
		return reagents.front();
	}
	cout<<"Please select a Reagent"<<endl;
	int ind=1;
	for(vector<Solution*>::const_iterator i=reagents.begin();i!=reagents.end();i++){
		cout<<::toString(ind,2)<<": "<<(*i)->toString()<<endl;
		ind++;
	}
	cout<<" a: abort"<<endl;
	string l=getString();
	if (l=="a")
		throw exception("Reagent selection aborted");
	ind=toInt(l);
	if (ind>reagents.size() || ind<1){
		logFile.write("invalid reagent selected.",true);
		return selectReagent();
	}
	return reagents.at(ind-1);
}


vector<Solution*> FluidicsSetup::selectReagents(){
	vector<Solution*> c;
	if (reagents.size()<1){
		throw exception("No reagents to select from");
	}
	if (reagents.size()==1){
		logFile.write("Only one reagent available:"+ reagents.front()->sd.name,true);
		c.push_back(reagents.front());
		return c;
	}
	cout<<"Please select reagents (e.g. 1-3,5,7)"<<endl;
	int ind=1;
	for(vector<Solution*>::const_iterator i=reagents.begin();i!=reagents.end();i++){
		cout<<::toString(ind,2)<<": "<<(*i)->toString()<<endl;
		ind++;
	}
	cout<<" a: abort"<<endl;
	string l=getString();
	if (l=="a")
		throw exception("Reagents selection aborted");
	vector<int> ints=toInts(l);
	for(vector<int>::iterator i=ints.begin();i!=ints.end();i++){
		if (*i>reagents.size() || *i<1){
			logFile.write("invalid reagent selected.",true);
			return selectReagents();
		}
		c.push_back(reagents.at(*i-1));
	}
	return c;
}

vector<Solution*> FluidicsSetup::selectSolutions(){
	vector<Solution*> c;
	if (solutions.size()<1){
		throw exception("No solutions to select from");
	}
	if (solutions.size()==1){
		logFile.write("Only one solution available:"+ solutions.front()->sd.name,true);
		c.push_back(solutions.front());
		return c;
	}
	cout<<"Please select solutions (e.g. 1-3,5,7)"<<endl;
	int ind=1;
	for(vector<Solution*>::const_iterator i=solutions.begin();i!=solutions.end();i++){
		cout<<::toString(ind,2)<<": "<<(*i)->toString()<<endl;
		ind++;
	}
	cout<<" a: abort"<<endl;
	string l=getString();
	if (l=="a")
		throw exception("Solutions selection aborted");
	vector<int> ints=toInts(l);
	for(vector<int>::iterator i=ints.begin();i!=ints.end();i++){
		if (*i>solutions.size() || *i<1){
			logFile.write("invalid solution selected.",true);
			return selectSolutions();
		}
		c.push_back(solutions.at(*i-1));
	}
	return c;
}

vector<FlowChannel*> FluidicsSetup::selectChannels(){
	vector<FlowChannel*> c;
	if (channels.size()<1){
		throw exception("No channels to select from");
	}
	if (channels.size()==1){
		logFile.write("Only one channel available:"+ channels.front()->name,true);
		c.push_back(channels.front());
		return c;
	}
	cout<<"Please select channels (e.g. 1-3,5,7)"<<endl;
	int ind=1;
	for(vector<FlowChannel*>::const_iterator i=channels.begin();i!=channels.end();i++){
		cout<<::toString(ind,2)<<": "<<(*i)->toString()<<endl;
		ind++;

	}
	cout<<" a: abort"<<endl;
	string l=getString();
	if (l=="a")
		throw exception("Channels selection aborted");
	vector<int> ints=toInts(l);
	for(vector<int>::iterator i=ints.begin();i!=ints.end();i++){
		if (*i>channels.size() || *i<1){
			logFile.write("invalid channel selected.",true);
			return selectChannels();
		}
		c.push_back(channels.at(*i-1));
	}
	return c;
}

FlowChannel* FluidicsSetup::selectChannel(){

	if (channels.size()<1){
		throw exception("No channels to select from");
	}
	if (channels.size()==1){
		logFile.write("Only one channel available:"+ channels.front()->name,true);
		return channels.front();
	}
	cout<<"Please select a Channel"<<endl;
	int ind=1;
	for(vector<FlowChannel*>::const_iterator i=channels.begin();i!=channels.end();i++){
		cout<<::toString(ind,2)<<": "<<(*i)->toString()<<endl;
		ind++;
	}
	cout<<" a: abort"<<endl;
	string l=getString();
	if (l=="a")
		throw exception("Channel selection aborted");
	ind=toInt(l);
	if (ind>channels.size() || ind<1){
		logFile.write("invalid channel selected.",true);
		return selectChannel();
	}
	return channels.at(ind-1);
}

FluidicsSetup::~FluidicsSetup(){
	for(vector<Solution*>::const_iterator i=solutions.begin();i!=solutions.end();i++){
		delete (*i);
	}
	for(vector<Solution*>::const_iterator i=reagents.begin();i!=reagents.end();i++){
		delete (*i);
	}
	for(vector<FlowChannel*>::const_iterator i=channels.begin();i!=channels.end();i++){
		delete (*i);
	}
}


bool FluidicsSetup::addSolution(Solution& s){
	for(vector<Solution*>::iterator i=solutions.begin();i!=solutions.end();i++){
		if (s.conflict(**i)){
			logFile.write("Cannot add "+s.toString()+". It conflicts with "+(*i)->toString(),true);
			return false;
		}
	}
	for(vector<Solution*>::iterator i=reagents.begin();i!=reagents.end();i++){
		if (s.conflict(**i)){
			logFile.write("Cannot add "+s.toString()+". It conflicts with "+(*i)->toString(),true);
			return false;
		}
	}
		solutions.push_back(s.clone(*this));
	return true;
}

bool FluidicsSetup::addReagent(Solution& s){
	for(vector<Solution*>::iterator i=solutions.begin();i!=solutions.end();i++){
		if (s.conflict(**i)){
			logFile.write("Cannot add "+s.toString()+". It conflicts with "+(*i)->toString(),true);
			return false;
		}
	}
	for(vector<Solution*>::iterator i=reagents.begin();i!=reagents.end();i++){
		if (s.conflict(**i)){
			logFile.write("Cannot add "+s.toString()+". It conflicts with "+(*i)->toString(),true);
			return false;
		}
	}
		reagents.push_back(s.clone(*this));
	return true;
}
bool FluidicsSetup::addChannel(FlowChannel& f){
	for(vector<FlowChannel*>::iterator i=channels.begin();i!=channels.end();i++){
		if (f.conflict(**i)){
			logFile.write("Cannot add "+f.toString()+". It conflicts with "+(*i)->toString(),true);
			return false;
		}
	}
	channels.push_back(new FlowChannel(f));
	return true;
}

FluidicsSetup& FluidicsSetup::operator=(const FluidicsSetup& fs){
	solutions.clear();
	reagents.clear();
	channels.clear();
	for(std::vector<Solution*>::const_iterator s=fs.solutions.begin();s!=fs.solutions.end();s++){
		solutions.push_back((*s)->clone(*this));
	}
	for(std::vector<Solution*>::const_iterator s=fs.reagents.begin();s!=fs.reagents.end();s++){
			reagents.push_back((*s)->clone(*this));
	}
	for(std::vector<FlowChannel*>::const_iterator s=fs.channels.begin();s!=fs.channels.end();s++){
		channels.push_back(new FlowChannel(**s));
	}
	return *this;
}

FluidicsSetup::FluidicsSetup(const FluidicsSetup& fs):solutions(),reagents(),channels(){
		for(std::vector<Solution*>::const_iterator s=fs.solutions.begin();s!=fs.solutions.end();s++){
			solutions.push_back((*s)->clone(*this));
		}
		for(std::vector<Solution*>::const_iterator s=fs.reagents.begin();s!=fs.reagents.end();s++){
			reagents.push_back((*s)->clone(*this));
		}
		for(std::vector<FlowChannel*>::const_iterator s=fs.channels.begin();s!=fs.channels.end();s++){
			channels.push_back(new FlowChannel(**s));
		}
}

string FluidicsSetup::toString(){
	string res;
	for(std::vector<Solution*>::const_iterator s=solutions.begin();s!=solutions.end();s++){
		res+=(*s)->toString()+"\n";
	}
	for(std::vector<FlowChannel*>::const_iterator s=channels.begin();s!=channels.end();s++){
		res+=(*s)->toString()+"\n";
	}
	return res;
}

void FluidicsSetup::fluidicsControl(){
	char c;
	string t;//temp string for input output
	string t2;
	string t3;
	string filename;
	int devnum;
	//Valve* v;
	Syringe* s;
	while(true){
		try{
			cout<<"Please select a task"<<endl;
			cout<<"1: Prime Solution(s)"<<endl;
			cout<<"2: Prime Reagent(s)"<<endl;
			cout<<"3: Wash Reagent Inlet Tubing"<<endl;
			cout<<"4: Quick Wash Reagent Inlet Tubing (also Air Inlet Tubing if appropriate)"<<endl;
			cout<<"6: Load Reagent"<<endl;
			cout<<"7: Wash Channel(s)"<<endl;
			cout<<"8: Load Reagent Partial (or complete partial load)"<<endl;
			cout<<"0: Put air into channels for chamber removal"<<endl;
			cout<<"c: Calibration Menu"<<endl;
			cout<<"f: Fine Control"<<endl;
			cout<<"h: Channel fine control"<<endl;
			cout<<"w: Test Wait Error"<<endl;
			cout<<"e: Exit Pump Control"<<endl;
			c=getChar();
			switch(c){
			case '1'://Prime Solution
				{
					vector<Solution*> v=selectSolutions();
					FlowChannel* f=NULL;
					
					for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
						(*i)->prime(f);
					}
				}
				break;
			case '2':// Prime Reagent 
				{
					vector<Solution*> v=selectReagents();
					FlowChannel* f=NULL;
					
					for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
						(*i)->prime(f);
					}
				}
				break;
			case '3'://wash reagent inlet tubing
				{
					vector<Solution*> v=selectReagents();
					if (v.empty()){
						logFile.write("No reagents selected",true);
						break;
					}
					cout<<"Ensure wash solution pushing out from reagent tubings will be collected in a suitable container (press enter to continue)"<<endl; 
					getString();
					cout<<"clean with custom solution first (e.g. NaOH) y or n?"<<endl;
					string s=getString();
					double speedFactor=10;
					double inletFactor=8;
					Solution* wash=NULL;
					if (s=="y"){
						wash=selectSolution();
						if (wash->sd.valveNum!=-1){
							logFile.write("Custom wash solution must be on syringe. Exiting",true);
							break;
						}
						wash->pull(0.75*wash->sd.s->volume,NULL,wash->sd.uLps*speedFactor);
						wash->sd.s->waste();
						for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
							(*i)->clean(inletFactor,wash->sd.uLps*speedFactor,NULL,wash);
						}	
						v.front()->sd.wash->pull(0.75*v.front()->sd.wash->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
						v.front()->sd.wash->sd.s->waste();
						v.front()->sd.wash->pull(0.75*v.front()->sd.wash->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
						v.front()->sd.wash->sd.s->waste();
						v.front()->sd.air->pull(0.75*v.front()->sd.air->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
						wash->sd.s->push(wash->sd.syringePort,wash->sd.inletTubingVol/2,wash->sd.uLps*5);
						wash->sd.s->waste();
						v.front()->sd.wash->pull(0.75*v.front()->sd.wash->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
						v.front()->sd.wash->sd.s->waste();
					}
					v.front()->sd.wash->pull(0.75*v.front()->sd.wash->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
					v.front()->sd.wash->sd.s->waste();
					for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
						(*i)->clean(inletFactor,(*i)->sd.wash->sd.uLps*speedFactor);
					}
					wash=this->getSolution("Water");
					if (wash!=NULL){
						logFile.write("Found water solution. Washing with water before filling tubing with air.",true);
						wash->pull(0.75*wash->sd.s->volume,NULL,wash->sd.uLps*speedFactor);
						wash->sd.s->waste();
						for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
							(*i)->clean(inletFactor,wash->sd.uLps*speedFactor,NULL,wash);
						}	
					}
					Solution* sAir=getSolution("AirSyringe");
					if (sAir==NULL){
						v.front()->sd.air->pull(0.75*v.front()->sd.air->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
						v.front()->sd.air->sd.s->waste();
						for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
							(*i)->clean(inletFactor,(*i)->sd.wash->sd.uLps*speedFactor,NULL,(*i)->sd.air);
						}
					}else{
						sAir->pull(0.75*sAir->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
						sAir->sd.s->waste();
						for(vector<Solution*>::iterator i=v.begin();i!=v.end();i++){
							(*i)->clean(inletFactor,(*i)->sd.wash->sd.uLps*speedFactor,NULL,sAir);
						}
					}
					if (v.front()->sd.waste==NULL){
						v.front()->sd.wash->pull(0.75*v.front()->sd.wash->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
						v.front()->sd.wash->sd.s->waste();
					}else{
						v.front()->sd.wash->pull(0.75*v.front()->sd.wash->sd.s->volume,NULL,v.front()->sd.wash->sd.uLps*speedFactor);
						v.front()->sd.waste->sd.s->push(v.front()->sd.waste->sd.syringePort,0.75*v.front()->sd.waste->sd.s->volume,v.front()->sd.waste->sd.uLps*speedFactor);
					}
					v.front()->sd.s->waste();
					
				}
				break;
				case '4'://wash reagent and reagent's air inlet tubing
				{
					Solution* v=selectReagent();
					Solution* sAir=getSolution("AirSyringe");
					if (v==NULL){
						logFile.write("No reagent selected",true);
						break;
					}
					if (sAir==NULL){
						logFile.write("Could not find AirSyringe",true);
						break;
					}
					if (v->sd.air->sd.inletTubingVol==0)
						cout<<"Reagent air has zero inlet tubing volume. Air port will not be washed"<<endl;
					cout<<"Ensure wash solution pushing out from reagent tubing and air port will be collected in a suitable container (press enter to continue)"<<endl; 
					getString();
					double speedFactor=10;
					double inletFactor=6;
					
					//suck out any extra solution first
					v->sd.s->waste();//(-1,NULL,s->sd.wash->sd.uLps*5.0);//push all the way out waste
					double vol,washVol;
					washVol=1.5*(v->sd.dv.getInjectTubingVolPull(v->sd.waste->sd.valveNum)-v->sd.dv.getInjectTubingVolPull(v->sd.valveNum));
					if (washVol==0)
						washVol=v->sd.inletTubingVol;
					vol=v->sd.inletTubingVol;
					v->pull(vol,NULL,v->sd.wash->sd.uLps*5.0);
					v->sd.wash->pull(washVol,NULL,v->sd.wash->sd.uLps*5.0);
					v->sd.waste->push(washVol+vol,NULL,v->sd.wash->sd.uLps*5.0);
					v->sd.s->waste();

					//clean with wash solution first
					v->sd.wash->pull(0.75*v->sd.wash->sd.s->volume,NULL,v->sd.wash->sd.uLps*speedFactor);
					v->sd.wash->sd.s->waste();
					v->clean(inletFactor,v->sd.wash->sd.uLps*speedFactor);
					//v->sd.air->clean(inletFactor,v->sd.wash->sd.uLps*speedFactor,NULL,v->sd.wash);
					
					//clean with water
					Solution* water=this->getSolution("Water");
					if (water!=NULL){
						logFile.write("Found water solution. Washing with water before filling tubing with air.",true);
						water->pull(0.75*water->sd.s->volume,NULL,water->sd.uLps*speedFactor);
						water->sd.s->waste();
						v->clean(inletFactor,water->sd.uLps*speedFactor,NULL,water);
						//v->sd.air->clean(inletFactor,water->sd.uLps*speedFactor,NULL,water);
					}

					//clean with air on syringe
					
					//sAir->pull(0.75*sAir->sd.s->volume,NULL,sAir->sd.uLps*speedFactor);
					//sAir->sd.s->waste();
					v->clean(inletFactor,v->sd.wash->sd.uLps*speedFactor,NULL,sAir);
					//v->sd.air->clean(inletFactor,v->sd.wash->sd.uLps*speedFactor,NULL,sAir);
					
					if (v->sd.waste==NULL){
						v->sd.wash->pull(0.75*v->sd.wash->sd.s->volume,NULL,v->sd.wash->sd.uLps*speedFactor);
						v->sd.wash->sd.s->waste();
					}else{
						v->sd.wash->pull(0.75*v->sd.wash->sd.s->volume,NULL,v->sd.wash->sd.uLps*speedFactor);
						v->sd.waste->valveSelect();
						v->sd.waste->sd.s->push(v->sd.waste->sd.syringePort,0.75*v->sd.waste->sd.s->volume,v->sd.waste->sd.uLps*speedFactor);
					}
					v->sd.s->waste();
					
				}
				break;
			case '6':// Load Reagent into channel(s)
				{
				Solution* r=selectReagent();
				vector<FlowChannel*> v=selectChannels();
				cout<<"Enter reaction time in seconds or -1 for manual wash"<<endl;
				double	time=getDouble();
					
					for(vector<FlowChannel*>::iterator i=v.begin();i!=v.end();i++){
						r->load(*i,-1,time);
					}
				break;
				}
			case '7'://Wash Channel(s)
				{
					Solution* s=selectSolution();
					vector<FlowChannel*> v=selectChannels();
					cout<<"Enter number of wash cycles to perform"<<endl;
					int num=getInt();
					double time=0;
					if (num>1){
						cout<<"Enter time (in min) to pause between cycles"<<endl;
						time=getDouble();
					}
					for (int j=0;j<num-1;j++){
					for(vector<FlowChannel*>::iterator i=v.begin();i!=v.end();i++){
						s->load(*i);
					}
					Timer::wait(1000.0*60.0*time);
					}
					for(vector<FlowChannel*>::iterator i=v.begin();i!=v.end();i++){
						s->load(*i);
					}
				}
				break;
			case '8'://partial load
				{
					Solution* s=this->selectReagent();
					if (s==NULL)
						break;
					if (s->sd.remainingLoadVolume!=0){
						double time=-1;
						cout<<"Enter reaction time in seconds or -1 for manual wash"<<endl;
						time=getDouble();
						s->load(NULL,-1,time);
						break;
					}
					FlowChannel* f=selectChannel();
					
					cout<<"Enter fraction (0-1) of channel to bring reagent front"<<endl; 
					double d=getDouble();
					s->load(f,d);
				}
				break;
			case '9':// Cal reagent inlet volume
				{
										
				}
				break;
			case '0':
				{
					Solution* s=this->getSolution("PushAir");
					if (s==NULL){
						logFile.write("No PushAir solution");
						break;
					
					}
					cout<<"select wash solution"<<endl;
					Solution* wash=selectSolution();
					//vector<FlowChannel*> v=selectChannels();
					SolutionData sd=s->sd;
					sd.air=s->clone(*this);//new creation
					sd.wash=wash;
					sd.loadFactor=3*sd.loadFactor;
					sd.waste=getSolution("Waste");
					sd.uLps=wash->sd.uLps;
					sd.air->sd.uLps=wash->sd.uLps;
					PushReagent r(sd);
					for(vector<FlowChannel*>::iterator i=this->channels.begin();i!=this->channels.end();i++){
						r.load(*i);
					}
					delete sd.air;
				}
			case 'c':
				calibrateControl();
				break;
			case 'f':// fine control
				{
					Solution* s=scan.fs.selectSolutionReagent();
					s->sd.s->fineControl(s->sd.syringePort,s->sd.uLps);
				}
				break;
			case 'h'://
				{
					FlowChannel* f=selectChannel();
					f->select();
					f->s->fineControl(f->syringePos,6);
				}
				break;
				case 'w'://
				{
					testWaitError();
				}
				break;
				case 'e':
				return;
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
			}
		}catch(std::ios_base::failure e)
		{
			logFile.write("Fluidics Control Menu Exception: "+string(e.what()),true);
			continue;
		}
	}
}

void FluidicsSetup::testWaitError(){
	Solution* s=selectSolution();
	cout<<"Select two flow channels to alternate between"<<endl;
	vector<FlowChannel*> v=selectChannels();
	cout<<"enter number of cycles"<<endl;
	int numTrials=getInt();
	Timer a(true);
	Timer t;
	double pullTimeSec=10;
	double pullVolFrac=0.75;
	
	if (v.size()<2) 
		return;
	int numErrors=0;
	v.front()->s->waste();
	for(int i=0;i<numTrials;i++){
		cout<<"Cycle "<<i+1<<" out of "<<numTrials<<" ("<<numErrors<<" errors so far)..."<<endl;
		//load first channel
		s->valveSelect();
		t.restart();
		s->sd.s->pull(s->sd.syringePort,s->sd.s->volume*pullVolFrac,s->sd.s->volume*pullVolFrac/pullTimeSec);
		if (t.getTime()<1000.0*pullTimeSec)
			numErrors++;
		v.front()->dvPull.selectOut();
		v.front()->dvPush.selectOut();
		v.front()->select();
		//load second channel
		v.front()->s->waste();
		s->valveSelect();
		t.restart();
		s->sd.s->pull(s->sd.syringePort,s->sd.s->volume*pullVolFrac,s->sd.s->volume*pullVolFrac/pullTimeSec);
		if (t.getTime()<1000.0*pullTimeSec)
			numErrors++;
		v.at(1)->dvPull.selectOut();
		v.at(1)->dvPush.selectOut();
		v.at(1)->select();
		//return syringe
		v.front()->s->waste();
	}
	double totalTime=a.getTime();
	cout<<"Number of errors: "<<numErrors<<" out of "<<numTrials*2<<" trials ("<<100.0*numErrors/(2*numTrials)<<"%)"<<endl;
	cout<<"Total runtime: "<<totalTime/1000.0/60<<" min ("<<totalTime/numTrials/1000.0<<" sec per cycle)"<<endl;
					
}

void FluidicsSetup::calibrateControl(){
	char c;
	Solution* s;
	while(true){
		try{
			cout<<"Please select a task"<<endl;
			cout<<"1: Coarse calibrate reagent inlet tubing volume"<<endl;
			cout<<"2: Fine calibrate reagent inlet tubing volume"<<endl;
			cout<<"3: Coarse calibrate daisy tubing volume"<<endl;
			cout<<"4: Fine calibrate daisy tubing volume"<<endl;
			cout<<"5: Coarse calibrate daisy valve inject/eject volume"<<endl;
			cout<<"6: Fine calibrate daisy valve inject/eject volume"<<endl;
			cout<<"7: Coarse calibrate flow channel injection volume"<<endl;
			cout<<"8: Fine calibrate flow channel injection volume using reagent and air gaps"<<endl;
			cout<<"9: Coarse calibrate solution inlet tubing volume"<<endl;
			cout<<"0: Calibrate tubing diameter"<<endl;
			cout<<"i: Prime reagent for test"<<endl;
			cout<<"j: Clean reagent inlet after test"<<endl;
			cout<<"e: Exit Calibration Control"<<endl;

			c=getChar();
			if (c==0)
				continue;
			if (c=='e')
				return;
			if (c=='9')
				s=this->selectSolution();
			else
				s=this->selectReagent();
			switch(c){
			case '1'://Coarse calibrate reagent inlet tubing volume
				{
					s->coarseCalibrateInlet();
				}
				break;
			case '2'://Fine calibrate reagent inlet tubing volume
				{
					s->calibrateInlet();
				}
				break;
			case '3'://Coarse calibrate daisy tubing volume
				{
					s->coarseCalibrateDV();
				}
				break;
			case '4'://Fine calibrate daisy tubing volume
				{
					s->calibrateDV();
				}
				break;
			case '5'://Coarse calibrate daisy valve inject/eject volume
				{
					s->coarseCalibrateDV2();
				}
				break;
			case '6'://Fine calibrate daisy valve inject/eject volume
				{
					s->calibrateDV2();
				}
				break;
			case '7'://Coarse calibrate flow channel injection volume
				{
					s->coarseCalibrateFlowChannel(this->selectChannel());
				}
				break;
			case '8'://Fine calibrate flow channel injection volume
				{
					s->calibrateFlowChannel2(this->selectChannel());
				}
				break;
			case '9'://Coarse calibrate solution inlet tubing volume
				{
					s->coarseCalibrateInlet();
				}
				break;
			case '0'://Coarse calibrate solution inlet tubing volume
				{
					s->calibrateTubingDiameter();
				}
				break;
			case 'i'://prime reagent
				{

					double speedFactor=1.0;
					s->sd.s->waste();
					double vol,washVol;
					washVol=1.5*(s->sd.dv.getInjectTubingVolPull(s->sd.waste->sd.valveNum)-s->sd.dv.getInjectTubingVolPull(s->sd.valveNum));
					if (washVol==0)
						washVol=s->sd.inletTubingVol;
					//put 1.5chambervolume into tube
					vol=1.5*s->sd.loadFactor*scan.cham.getChannelVolume();
					s->sd.wash->pull(vol,NULL,s->sd.wash->sd.uLps*5.0);
					s->push(vol);

					//put air into tube and clean daisy tubing
					vol=s->sd.inletTubingVol;
					s->sd.wash->pull(washVol,NULL,s->sd.wash->sd.uLps*5.0);
					s->sd.air->pull(2.0*vol);
					s->push(1.5*vol);
					s->sd.waste->push(-1,NULL,s->sd.wash->sd.uLps*5.0);

					cout<<"Clean tip of "<<s->toString()<< ", centrifuge tube and reinsert tubing before priming. An air bubble at the bottom of the tube will ruin calibration (press enter to continue)"<<endl;
					getString();
					s->prime(NULL);
				}
				break;
			case 'j'://
				{
					s->sd.s->waste();//(-1,NULL,s->sd.wash->sd.uLps*5.0);//push all the way out waste
					double vol,washVol;
					washVol=1.5*(s->sd.dv.getInjectTubingVolPull(s->sd.waste->sd.valveNum)-s->sd.dv.getInjectTubingVolPull(s->sd.valveNum));
					if (washVol==0)
						washVol=s->sd.inletTubingVol;
					vol=s->sd.inletTubingVol;
					s->pull(vol,NULL,s->sd.wash->sd.uLps*5.0);
					s->sd.wash->pull(washVol,NULL,s->sd.wash->sd.uLps*5.0);
					s->sd.waste->push(washVol+vol,NULL,s->sd.wash->sd.uLps*5.0);
					s->sd.s->waste();
				}
				break;
			default:
				cout<<c<<" is not a valid option"<<endl<<endl;
			}
		}catch(std::ios_base::failure e)
		{
			logFile.write("Calibrate Control Menu Exception: "+string(e.what()),true);
			continue;
		}
	}
}
