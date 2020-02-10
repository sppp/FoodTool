#ifndef _FoodTool_Util_h_
#define _FoodTool_Util_h_


struct RunningFlag {
	int sleep_time = 100;
	bool running = false;
	Atomic workers_running;
	
	RunningFlag() {workers_running = 0;}
	void Start(int count) {Stop(); running = true; workers_running = count;}
	void Stop() {running = false; while (workers_running > 0) Sleep(sleep_time);}
	void SetNotRunning() {running = false;}
	void IncreaseStopped() {workers_running--;}
	bool IsRunning() const {return running;}
};



#ifdef flagWIN32
inline void PlayCameraShutter() {PlaySoundA(ConfigFile("camera-shutter.wav"), NULL, SND_ASYNC|SND_FILENAME);}
inline void PlayAlert() {PlaySoundA(ConfigFile("alert.wav"), NULL, SND_ASYNC|SND_FILENAME);}
#else
inline void PlayCameraShutter() {}
inline void PlayAlert() {}
#endif

Color Rainbow(float progress);



enum {
	DEF_CALCIUM,
	DEF_MAGNESIUM,
	DEF_SODIUM,
	DEF_POTASSIUM,
	DEF_COPPER,
	DEF_ZINC,
	DEF_MANGANESE,
	DEF_CHROMIUM,
	DEF_VANADIUM,
	DEF_MOLYBDENUM,
	DEF_BORON,
	DEF_IODINE,
	DEF_LITHIUM,
	DEF_PHOSPHORUS,
	DEF_SELENIUM,
	DEF_STRONTIUM,
	DEF_SULFUR,
	DEF_COBALT,
	DEF_IRON,
	DEF_GERMANIUM,
	DEF_RUBIDIUM,
	DEF_ZIRCONIUM,
	
	DEF_COUNT
};

String GetNutrientDeficitElement(int i);

struct NutrientDeficitProfile : Moveable<NutrientDeficitProfile> {
	double elements[DEF_COUNT];
	Time added;
	
	void Reset() {
		for(int i = 0; i < DEF_COUNT; i++)
			elements[i] = 50;
	}
	void Serialize(Stream& s) {
		for(int i = 0; i < DEF_COUNT; i++)
			s % elements[i];
		s % added;
	}
};

struct NutrientDeficitEdit : public Ctrl {
	NutrientDeficitProfile prof;
	Vector<double> x_to_percentile;
	double x25, x160, x450, x500, x550, x840, x975;
	
	Color green, yellow, red;
	
	NutrientDeficitEdit();
	virtual void Paint(Draw& d);
	virtual void LeftDown(Point p, dword keyflags);
};

#endif
