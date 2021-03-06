#include "Food.h"


int KCAL, PROT, FAT, CARB, SODIUM, PUFA;



Database::Database() {
	LoadThis();
}

void LoadLine(String line, Vector<String>& items) {
	items.SetCount(0);
	
	enum {
		EXPECTING_ITEM,
		READING_ITEM,
		SIMPLE_READ,
	};
	
	int mode = EXPECTING_ITEM;
	String* it = NULL;
	for(int i = 0; i < line.GetCount(); i++) {
		int chr = line[i];
		int peek_chr = i+1 < line.GetCount() ? line[i+1] : 0;
		
		if (mode == EXPECTING_ITEM) {
			if (chr == '~') {
				mode = READING_ITEM;
				it = &items.Add();
			}
			else if (chr == '^')
				items.Add(); // empty item
			else {
				mode = SIMPLE_READ;
				it = &items.Add();
				it->Cat(chr);
			}
		}
		else if (mode == READING_ITEM) {
			if (chr == '~' && peek_chr == '^') {
				i++;
				mode = EXPECTING_ITEM;
			}
			else if (chr == '~' && peek_chr == 0) {
				break;
			}
			else {
				it->Cat(chr);
			}
		}
		else if (mode == SIMPLE_READ) {
			if (chr == '^')
				mode = EXPECTING_ITEM;
			else
				it->Cat(chr);
		}
	}
	if (mode == EXPECTING_ITEM)
		items.Add();
}




bool Database::Init() {
	String dir = ConfigFile("db");
	
	//String data_src = LoadFile(AppendFileName(dir, "DATA_SRC.txt"));
	//String datsrcln = LoadFile(AppendFileName(dir, "DATSRCLN.txt"));
	//String deriv_cd = LoadFile(AppendFileName(dir, "DERIV_CD.txt"));
	String fd_group = LoadFile(AppendFileName(dir, "FD_GROUP.txt"));
	String food_des = LoadFile(AppendFileName(dir, "FOOD_DES.txt"));
	//String footnote = LoadFile(AppendFileName(dir, "FOOTNOTE.txt"));
	//String langdesc = LoadFile(AppendFileName(dir, "LANGDESC.txt"));
	//String langual = LoadFile(AppendFileName(dir, "LANGUAL.txt"));
	String nut_data = LoadFile(AppendFileName(dir, "NUT_DATA.txt"));
	String nutr_def = LoadFile(AppendFileName(dir, "NUTR_DEF.txt"));
	//String src_cd = LoadFile(AppendFileName(dir, "SRC_CD.txt"));
	//String weight = LoadFile(AppendFileName(dir, "WEIGHT.txt"));
	
	Vector<String> items;
	
	
	
	{
		Vector<String> lines = Split(fd_group, "\r\n");
		for(String line : lines) {
			LoadLine(line, items);
			
			FoodGroup& g = food_groups.Add(items[0]);
			g.name = items[1];
		}
		FoodGroup& other = food_groups.Add("OTHER");
		other.name = "Other";
	}
	
	Vector<int> nutr_no_trans;
	nutr_no_trans.SetCount(1000, -1);
	{
		Vector<String> lines = Split(nutr_def, "\r\n");
		for(String line : lines) {
			LoadLine(line, items);
			
			int no = StrInt(items[0]);
			nutr_no_trans[no] = nutr_types.GetCount();
			
			NutritionType& t = nutr_types.Add();
			t.is_valid = true;
			t.units = items[1];
			t.tagname = items[2];
			t.nutr_desc = items[3];
		}
	}
	
	{
		Vector<String> lines = Split(food_des, "\r\n");
		for(String line : lines) {
			LoadLine(line, items);
			//DUMPC(items);
			
			FoodDescription& s = food_descriptions.Add(items[0]);
			s.food_group_key = items[1];
			s.long_desc = items[2];
			s.short_desc = items[3];
			s.company_name = items[4];
			s.manufacturer_name = items[5];
			s.survey = items[6];
			s.ref_desc = items[7];
			s.refuse = StrInt(items[8]);
			s.sci_name = items[9];
			s.n_factor = StrDbl(items[10]);
			s.pro_factor = StrDbl(items[11]);
			s.fat_factor = StrDbl(items[12]);
			s.cho_factor = StrDbl(items[13]);
		}
	}
	
	{
		Vector<String> lines = Split(nut_data, "\r\n");
		for(String line : lines) {
			LoadLine(line, items);
			
			FoodDescription& s = food_descriptions.Get(items[0]);
			NutritionInfo& n = s.nutr.Add();
			
			n.nutr_no = nutr_no_trans[StrInt(items[1])];
			n.nutr_value = StrDbl(items[2]);
			n.std_error = StrDbl(items[4]);
			ASSERT(n.nutr_no < nutr_types.GetCount());
		}
	}
	
	AddRecommendation("PROCNT", 0.8, true, OTHER);
	AddRecommendation("FAT", 60.5 / 70.0, true, OTHER);
	AddRecommendation("CA", 1000 / 70.0, true, MINERAL);
	AddRecommendation("FE", 19.8 / 70.0, true, MINERAL);
	AddRecommendation("MG", 420 / 70.0, true, MINERAL);
	AddRecommendation("P", 700 / 70.0, true, MINERAL);
	AddRecommendation("K", 4100 / 70.0, true, MINERAL);
	AddRecommendation("NA", 1500 / 70.0, true, MINERAL);
	AddRecommendation("ZN", 11 / 70.0, true, MINERAL);
	AddRecommendation("CU", 1 / 70.0, true, MINERAL);
	//AddRecommendation("FLD", 50, true);
	AddRecommendation("MN", 2.3, true, MINERAL);
	AddRecommendation("SE", 100 / 70.0, true, MINERAL);
	AddRecommendation("VITA_IU", 3000, true, VITAMIN);
	AddRecommendation("RETOL", 900 / 70.0, true, VITAMIN);
	AddRecommendation("VITA_RAE", 900 / 70.0, true, VITAMIN);
	AddRecommendation("CARTB", 15000 / 70.0, true, VITAMIN);
	AddRecommendation("CARTA", 600 / 70.0, true, VITAMIN);
	AddRecommendation("TOCPHA", 15 / 70.0, true, VITAMIN);
	//AddRecommendation("VITD", 800 / 70.0, true); // use D2+D3 instead
	AddRecommendation("VITDMCR", 100 / 70.0, true, VITAMIN);
	AddRecommendation("LYCPN", 21000 / 70.0, true, VITAMIN);
	AddRecommendation("VITC", 90 / 70.0, true, VITAMIN);
	AddRecommendation("THIA", 1.2 / 70.0, true, VITAMIN);
	AddRecommendation("RIBF", 3 / 70.0, true, VITAMIN);
	AddRecommendation("NIA", 16 / 70.0, true, VITAMIN);
	AddRecommendation("PANTAC", 5 / 70.0, true, VITAMIN);
	AddRecommendation("VITB6A", 1.3 / 70.0, true, VITAMIN);
	AddRecommendation("FOL", 400 / 70.0, true, VITAMIN);
	AddRecommendation("VITB12", 1.8 / 70.0, true, VITAMIN);
	AddRecommendation("CHOLN", 500 / 70.0, true, VITAMIN);
	//AddRecommendation("MK4", 92.5 / 70.0, true); // bad for you
	//AddRecommendation("VITK1D", 0 / 70.0, true); // bad for you
	AddRecommendation("FOLAC", 400 / 70.0, true, VITAMIN);
	AddRecommendation("BETN", 550 / 70.0, true, VITAMIN);
	AddRecommendation("TRP_G", 0.006, true, AMINOACID);
	AddRecommendation("THR_G", 0.015, true, AMINOACID);
	AddRecommendation("ILE_G", 1.4 / 70.0, true, AMINOACID);
	AddRecommendation("LEU_G", 2.7 / 70.0, true, AMINOACID);
	AddRecommendation("LYS_G", 1 / 70.0, true, AMINOACID);
	AddRecommendation("MET_G", 0.019, true, AMINOACID);
	AddRecommendation("PHE_G", 0.026, true, AMINOACID);
	AddRecommendation("TYR_G", 6 / 70.0, true, AMINOACID);
	AddRecommendation("VAL_G", 1.8 / 70.0, true, AMINOACID);
	AddRecommendation("ARG_G", 6 / 70.0, true, AMINOACID);
	AddRecommendation("HISTN_G", 1 / 70.0, true, AMINOACID);
	AddRecommendation("ALA_G", 3.5 / 70.0, true, AMINOACID);
	AddRecommendation("ASP_G", 3 / 70.0, true, AMINOACID);
	AddRecommendation("GLU_G", 15 / 70.0, true, AMINOACID);
	//AddRecommendation("GLY_G", 5 / 70.0, true);
	AddRecommendation("GLY_G", 5 / 70.0, true, AMINOACID);
	AddRecommendation("VITK1", 120 / 70.0, true, VITAMIN);
	AddRecommendation("PRO_G", 1 / 70.0, true, AMINOACID);
	AddRecommendation("CHOLE", 300 / 70.0, true, OTHER);
	
	/*
	Important 'need to know':
		- Only small dense LDL colesterol (sdLDL-c) is associated with metabolic syntrome.
		  All LDL is not 'bad', even though majority of public information states so.
		  (https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6341753/)
		- sdLDL-c is highly correlated by triglyceride levels
		  (https://www.ncbi.nlm.nih.gov/pmc/articles/PMC5270205/)
		- The causal relation between triglyceride and sdLDL-c IS NOT DETERMINED (1Q 2020).
		  The assumed sdLDL-c --> triglyceride causality is NOT proven nor even studied.
		  (lack of sources)
		- Starch, sugar and excess calories creates excess triglycerides.
		  (e.g. https://healthyeating.sfgate.com/conversion-carbohydrates-triglycerides-2218.html)
		- Reduction of trans- and saturated fat is not valid by "LDL is bad" argument
		- Trans-fats have anti-cancer properties, and the global reduction of usage of trans
		  and saturated fatty acids might be the partial reason for growing cancer rates.
		- official recommendation of 2-5.8g trans fats and 5-6g saturates fats IS MURDER!
		- waist circumference and body fat percentage is best predictor of MetS
		  (https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4450687/)
		- Triglycerides are released from cells by glucagon, when the body requires fatty acids as an
		  energy source. Insulin causes triglycerides to be taken back to cells. You have too
		  much triglycerides in your blood when cells cannot absorb any more, e.g. when you
		  have eaten too much.
		  (https://en.wikipedia.org/wiki/Triglyceride)
		  (https://en.wikipedia.org/wiki/Fatty_acid_metabolism)
		  
		----> The root of the problem is excess calories and pharmaceutical industry
			  shamelessly spread misleading information to sell all sorts of medicine an other:
			    - diabetes medicine
			    - ineffective cancer treatment
			    - dementia medicine
				- causing erucic acid ban
						Eurcic acid actually causes weight loss by releasing triglycerides from cells
						allowing you to eat less and lose weight faster, but if you don't "spend"
						those it will increase sdLDL-c, of course...
						This is banned because it is too healthy for people's stupid habits.
				
	*/
	
	double mul = 1;
	double total = 130 / 70.0 * mul;
	double fat_trans = total / 3;
	double fat_saturated = total / 3;
	double fat_unsaturated = total / 3;
	AddRecommendation("FATRN", fat_trans, true, OTHER);
	AddRecommendation("FASAT", fat_saturated, true, OTHER);
	AddRecommendation("FAMS", fat_unsaturated / 2, true, OTHER);
	AddRecommendation("FAPU", fat_unsaturated / 2, true, OTHER);
	AddRecommendation("FATRNM", 0.873 * fat_trans, true, OTHER);
	AddRecommendation("FATRNP", 0.126 * fat_trans, true, OTHER);
	AddRecommendation("FIBTG", 30 / 70.0, true, OTHER);
	
	// https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6769731/
	double fat_saturated_part = 1.0 / 12;
	AddRecommendation("F4D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F6D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F8D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F10D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F12D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F14D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F15D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F16D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F17D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F18D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F20D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	AddRecommendation("F24D0", fat_saturated * fat_saturated_part, true, FATTYACID);
	
	double fat_unsaturated_part = 1.0 / (7 + 12 + 7);
	AddRecommendation("F14D1", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F15D1", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F16D1", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F18D1", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F20D1", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F22D1", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F22D1", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	
	AddRecommendation("F24D1C", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F16D1C", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F18D1C", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F18D3CN6", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F18D3CN3", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F20D3N3", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F20D3N6", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F20D4N6", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F22D4", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F18D2CN6", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F18D2CLA", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F22D1C", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	
	AddRecommendation("F18D2", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F18D3", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F20D4", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F22D6", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F18D4", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F20D5", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	AddRecommendation("F22D5", fat_unsaturated * fat_unsaturated_part, true, FATTYACID);
	
	double fat_trans_part = 1.0 / 2;
	AddRecommendation("F16D1T", fat_trans * 0.01 * fat_trans_part, true, FATTYACID);
	AddRecommendation("F18D2TT", fat_trans * 0.01 * fat_trans_part, true, FATTYACID);
	AddRecommendation("F22D1T", 0.015 / 70, true, FATTYACID);
	AddRecommendation("F18D1T", 0.001, false, FATTYACID);
	
	
	
	#if 0
	
	
	// https://wholefoodcatalog.info/nutrient/vitamin_b7(biotin)/vegetables/
	int b7 = AddNutrition("B7-Vitamin Biotin", "mg", "");
	// broccoli 13.5ug / 290g
	// cauli 28.1ug / 660g
	// pea 0.3ug / 5g
	// spinach 7.6ug / 250g
	// lettuce 7ug / 257g
	// carrot 3.6ug / 130g
	// eggplant 3.3ug / 160g
	// tomato 3.9ug / 220g
	// garlic 1ug / 55g
	// asparagus 1.4ug / 95g
	// cabbage 23.ug / 1700g
	// chinese cabbage 27.6ug /2100g
	// cucumber 1.6ug / 120g
	// onion 1ug / 200g
	
	int chrom = AddNutrition("Chromium", "mg", "");
	// Broccoli	1 cup	54.6	18.55ug
	// Green Beans	1 cup	43.8	2.04ug
	// Tomatoes	1 cup	32.4	1ug
	
	int moly = AddNutrition("Molybdenum", "mg", "");
	// Lentils	1 cup	229.7	148.50ug
	// Kidney Beans	1 cup	224.8	132.75ug
	// Soybeans	1 cup	297.6	129.00ug
	// Black Beans	1 cup	227.0	129.00ug
	// Lima Beans	1 cup	216.2	141.00ug
	// Tomatoes	1 cup	32.4	9.00ug
	// Cucumber	1 cup	15.6	5.20ug
	// Carrots	1 cup	50.0	6.10ug
	// Bell Peppers	1 cup	28.5	4.60ug
	// Peanuts	0.25 cup	206.9	10.77ug
	// Sesame Seeds	0.25 cup	206.3	10.62ug
	// Walnuts	0.25 cup	196.2	8.85ug
	// Green Peas	1 cup	115.7	6.89ug
	// Almonds	0.25 cup	132.2	6.78ug
	
	// https://www.tandfonline.com/doi/pdf/10.1080/10942910701584252
	int sili = AddNutrition("Silicon", "mg", "");
	// beans 5577ug / 100g
	// spinach 1782ug / 100g
	// banana 6195ug / 100g
	
	// http://cookcal.info/en/foodstuffs/content/348
	int brom = AddNutrition("Bromine", "mg", "");
	// Ginger 0.0468mg / 100g
	// Lentils 0.0033mg / 100g
	
	// https://www.torrinomedica.it/approfondimenti/dietologia/nickel-in-foods/
	int nick = AddNutrition("Nickel", "mg", "");
	// Almonds	130.0ug /100g
	// Apple	2.0ug /100g
	// Apricots, dried	120.0ug /100g
	// Avocado	100.0ug /100g
	// Banana	7.0ug /100g
	// Beets	5.0ug /100g
	// Blackcurrant	10.0ug /100g
	// Broccoli	10.0ug /100g
	// Brussels Sprouts	3.0ug /100g
	// Cabbages	4.0ug /100g
	// Carrots	5.0ug /100g
	// Cauliflower	4.0ug /100g
	// Chickpeas, dried	130.0ug /100g
	// Cocoa powder	855.0ug /100g
	// Hazelnuts	170.0ug /100g
	// Lentils, dried	248.0ug /100g
	// Olives	9.0ug /100g
	// Onions	3.0ug /100g
	// Orange	4.0ug /100g
	// Peanuts	390.0ug /100g
	// Peas	22.0ug /100g
	// Pineapple	50.0ug /100g
	// Red lentils, dried	292.0ug /100g
	// Soybean oil	6.0ug /100g
	// Spinach	3.0ug /100g
	// Sunflower seeds	356.0ug /100g
	// Tomatoes	10.0ug /100g
	// Walnut	320.0ug /100g
	
	// https://www.greenfacts.org/en/boron/toolboxes/2.htm
	int boron = AddNutrition("Boron", "mg", "");
	// mg/ kg <-- !!!
	// Apple, red with peel, raw 	2.73
	// Banana, raw 	- 	3.72 
	// Dates 	9.2
	// Beans, green 		1.56
	// Broccoli, flowers 	1.85 
	// Lettuce, iceberg 	<0.015
	// Carrots, canned 	0.75 
	// Almonds 	23
	// Hazelnuts 	16
	// Peanuts 	18
	
	// http://cookcal.info/en/foodstuffs/content/427?group=30
	int lith = AddNutrition("Lithium", "mg", "");
	// Potatoes 0.077mg/100g
	// Tomatoes 0.029mg/100g
	// Carrot  0.006mg/100g
	// Ginger 0.00079mg/100g
	
	#endif
	
	
	is_init = true;
	
	StoreThis();
	
	return true;
}

int Database::AddNutrition(String name, String unit, String desc) {
	int i = nutr_types.GetCount();
	NutritionType& t = nutr_types.Add();
	t.is_valid = true;
	t.is_user_added = true;
	t.tagname = name;
	t.units = unit;
	t.nutr_desc = desc;
	return i;
}

FoodDescription& Database::AddFood(String fg, String l, String s, String c, String m, String su, String r, int re, String sci, double nf, double pf, double ff, double cf) {
	String key;
	for(int i = 0; i < 8; i++)
		key.Cat('A' + Random('Z' - 'A' + 1));
	
	FoodDescription& d = food_descriptions.Add(key);
	d.food_group_key = fg;
	d.long_desc = l;
	d.short_desc = s;
	d.company_name = c;
	d.manufacturer_name = m;
	d.survey = su;
	d.ref_desc = r;
	d.refuse = re;
	d.sci_name = sci;
	d.n_factor = nf;
	d.pro_factor = pf;
	d.fat_factor = ff;
	d.cho_factor = cf;
	d.is_user_added = true;
	return d;
}

FoodDescription& FoodDescription::AddNutrition(int nutr_no, double value, double error) {
	NutritionInfo& i = nutr.Add();
	i.nutr_no = nutr_no;
	i.nutr_value = value;
	i.std_error = error;
	return *this;
}

void Database::VLCD_Preset() {
	
	
	AddFood("OTHER", "Salt, iodized", "", "Meira", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(25, 38758, 0);
	AddFood("0100", "Magnex sitraatti + B6- vitamiini", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(29, 12254, 0)
		.AddNutrition(56, 98, 0);
	AddFood("0100", "Reformi 55g B-vitamiini", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(52, 6363, 0)
		.AddNutrition(53, 6363, 0)
		.AddNutrition(54, 6363, 0)
		.AddNutrition(55, 6363, 0)
		.AddNutrition(56, 4545, 0)
		.AddNutrition(58, 1818, 0)
		.AddNutrition(63, 109090, 0);
	AddFood("0100", "Calcia 800 Plus kalkki-monivitamiini-monimineraalitabletti", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(56, 350.428588867187, 0)
		.AddNutrition(51, 9557.142578125, 0)
		.AddNutrition(40, 1592.85717773438, 0)
		.AddNutrition(62, 11150, 0)
		.AddNutrition(20, 127428.5703125, 0)
		.AddNutrition(63, 47785.71484375, 0)
		.AddNutrition(22, 55750, 0)
		.AddNutrition(29, 398.214294433594, 0)
		.AddNutrition(51, 9557.142578125, 0)
		.AddNutrition(26, 2389.28564453125, 0);
	AddFood("0100", "Rainbow Vahva C-vitamiini 500 mg 90 tablettia", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(51, 64935, 0);
	AddFood("0100", "Rainbow 72g monivitamiini-hivenainetabletti", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(53, 115.200004577637, 0)
		.AddNutrition(52, 100.799995422363, 0)
		.AddNutrition(58, 180, 0)
		.AddNutrition(56, 158.400009155273, 0)
		.AddNutrition(51, 5760, 0)
		.AddNutrition(40, 720, 0)
		.AddNutrition(44, 0.864000022411346, 0)
		.AddNutrition(22, 8640, 0)
		.AddNutrition(29, 144, 0)
		.AddNutrition(54, 1152, 0)
		.AddNutrition(55, 432, 0)
		.AddNutrition(30, 3960, 0)
		.AddNutrition(51, 5760, 0)
		.AddNutrition(40, 720, 0)
		.AddNutrition(36, 864, 0)
		.AddNutrition(26, 720, 0);
	AddFood("0100", "Makrobios B-12 vitamiini+folaatti ", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(58, 285000, 0)
		.AddNutrition(63, 57142, 0);
	AddFood("0100", "Reformi Multivitamin 100tabl ", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(40, 510, 0)
		.AddNutrition(36, 1020, 0)
		.AddNutrition(51, 6122, 0)
		.AddNutrition(52, 142, 0)
		.AddNutrition(53, 163, 0)
		.AddNutrition(54, 1836, 0)
		.AddNutrition(56, 224, 0)
		.AddNutrition(63, 30612, 0)
		.AddNutrition(58, 306, 0)
		.AddNutrition(55, 408, 0)
		.AddNutrition(20, 20408, 0)
		.AddNutrition(22, 9183, 0)
		.AddNutrition(26, 1530, 0)
		.AddNutrition(30, 5102, 0);
	AddFood("0100", "Nutrilet", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(4, 363, 0)
		.AddNutrition(1, 7.5, 0)
		.AddNutrition(2, 31, 0)
		.AddNutrition(0, 40, 0)
		.AddNutrition(25, 1840, 0)
		.AddNutrition(33, 965, 0)
		.AddNutrition(40, 3.40000009536743, 0)
		.AddNutrition(36, 6.09999990463257, 0)
		.AddNutrition(62, 57, 0)
		.AddNutrition(51, 51, 0)
		.AddNutrition(52, 1, 0)
		.AddNutrition(53, 1.20000004768372, 0)
		.AddNutrition(54, 13, 0)
		.AddNutrition(56, 1.39999997615814, 0)
		.AddNutrition(63, 170, 0)
		.AddNutrition(58, 2.20000004768372, 0)
		.AddNutrition(55, 3.90000009536743, 0)
		.AddNutrition(20, 643, 0)
		.AddNutrition(24, 1390, 0)
		.AddNutrition(22, 263, 0)
		.AddNutrition(30, 34, 0);
	AddFood("OTHER", "Rainbow Omega 3+ ADE kalaöljy-vitamiinikapseli, 100 kapselia 94 g", "", "", "", "", "", 0, "", 0, 0, 0, 0)
		.AddNutrition(1, 67, 0)
		.AddNutrition(90, 10.6000003814697, 0)
		.AddNutrition(2, 10, 0)
		.AddNutrition(0, 18, 0)
		.AddNutrition(33, 54794, 0)
		.AddNutrition(40, 1369, 0)
		.AddNutrition(36, 1369, 0)
		.AddNutrition(4, 693, 0);
	used_foods.Add(15);
	used_foods.Add(16);
	used_foods.Add(117);
	used_foods.Add(239);
	used_foods.Add(261);
	used_foods.Add(266);
	used_foods.Add(276);
	used_foods.Add(278);
	used_foods.Add(289);
	used_foods.Add(291);
	used_foods.Add(294);
	used_foods.Add(657);
	used_foods.Add(658);
	used_foods.Add(659);
	used_foods.Add(661);
	used_foods.Add(662);
	used_foods.Add(685);
	used_foods.Add(716);
	used_foods.Add(725);
	used_foods.Add(883);
	used_foods.Add(1057);
	used_foods.Add(2122);
	used_foods.Add(2150);
	used_foods.Add(2155);
	used_foods.Add(2158);
	used_foods.Add(2165);
	used_foods.Add(2190);
	used_foods.Add(2194);
	used_foods.Add(2222);
	used_foods.Add(2240);
	used_foods.Add(2252);
	used_foods.Add(2254);
	used_foods.Add(2270);
	used_foods.Add(2275);
	used_foods.Add(2280);
	used_foods.Add(2283);
	used_foods.Add(2299);
	used_foods.Add(2304);
	used_foods.Add(2307);
	used_foods.Add(2310);
	used_foods.Add(2325);
	used_foods.Add(2339);
	used_foods.Add(2352);
	used_foods.Add(2358);
	used_foods.Add(2373);
	used_foods.Add(2376);
	used_foods.Add(2384);
	used_foods.Add(2392);
	used_foods.Add(2423);
	used_foods.Add(2596);
	used_foods.Add(2666);
	used_foods.Add(2823);
	used_foods.Add(2826);
	used_foods.Add(2836);
	used_foods.Add(2862);
	used_foods.Add(2870);
	used_foods.Add(2878);
	used_foods.Add(2887);
	used_foods.Add(2900);
	used_foods.Add(2907);
	used_foods.Add(2923);
	used_foods.Add(2938);
	used_foods.Add(2957);
	used_foods.Add(2961);
	used_foods.Add(2967);
	used_foods.Add(2968);
	used_foods.Add(2982);
	used_foods.Add(3000);
	used_foods.Add(3001);
	used_foods.Add(3002);
	used_foods.Add(3008);
	used_foods.Add(3009);
	used_foods.Add(3029);
	used_foods.Add(3051);
	used_foods.Add(3079);
	used_foods.Add(3094);
	used_foods.Add(3150);
	used_foods.Add(3166);
	used_foods.Add(3179);
	used_foods.Add(3187);
	used_foods.Add(3222);
	used_foods.Add(3238);
	used_foods.Add(3295);
	used_foods.Add(3305);
	used_foods.Add(3322);
	used_foods.Add(3435);
	used_foods.Add(3437);
	used_foods.Add(3490);
	used_foods.Add(3578);
	used_foods.Add(3609);
	used_foods.Add(3619);
	used_foods.Add(3625);
	used_foods.Add(3634);
	used_foods.Add(3644);
	used_foods.Add(3663);
	used_foods.Add(3665);
	used_foods.Add(3689);
	used_foods.Add(3715);
	used_foods.Add(4262);
	used_foods.Add(4431);
	used_foods.Add(4752);
	used_foods.Add(4754);
	used_foods.Add(4765);
	used_foods.Add(4775);
	used_foods.Add(4778);
	used_foods.Add(4787);
	used_foods.Add(4794);
	used_foods.Add(4806);
	used_foods.Add(4808);
	used_foods.Add(4817);
	used_foods.Add(4824);
	used_foods.Add(4853);
	used_foods.Add(4858);
	used_foods.Add(4875);
	used_foods.Add(5610);
	used_foods.Add(5880);
	used_foods.Add(6140);
	used_foods.Add(6194);
	used_foods.Add(6452);
	used_foods.Add(6496);
	used_foods.Add(6560);
	used_foods.Add(6582);
	used_foods.Add(6616);
	used_foods.Add(6621);
	used_foods.Add(7615);
	used_foods.Add(8604);
	used_foods.Add(8681);
	used_foods.Add(8789);
	used_foods.Add(8790);
	used_foods.Add(8791);
	used_foods.Add(8792);
	used_foods.Add(8793);
	used_foods.Add(8794);
	used_foods.Add(8795);
	used_foods.Add(8796);
	used_foods.Add(8797);
	used_foods.Add(8798);
	used_foods.Add(646);
	used_foods.Add(0);
	used_foods.Add(4557);
	used_foods.Add(2853);
	used_foods.Add(292);
	used_foods.Add(3038);
	used_foods.Add(4860);
	used_foods.Add(2987);
	used_foods.Add(308);
	used_foods.Add(111);
	used_foods.Add(2930);
	used_foods.Add(1517);
	used_foods.Add(3568);
	used_foods.Add(5849);
	used_foods.Add(8);
	used_foods.Add(6093);
	used_foods.Add(280);
	used_foods.Add(879);
	used_foods.Add(309);
	used_foods.Add(18);
	used_foods.Add(6754);
	used_foods.Add(739);
	used_foods.Add(8690);
	used_foods.Add(1673);
	used_foods.Add(6127);
	used_foods.Add(6083);
	used_foods.Add(156);
	used_foods.Add(263);
	used_foods.Add(3086);
	used_foods.Add(1726);
	used_foods.Add(1292);
	used_foods.Add(269);
	used_foods.Add(285);
	used_foods.Add(4546);
	used_foods.Add(2905);
	used_foods.Add(3379);
	used_foods.Add(3391);
	used_foods.Add(4627);
	used_foods.Add(3206);
	used_foods.Add(3927);
	used_foods.Add(25);
	used_foods.Add(17);
	used_foods.Add(6507);
	used_foods.Add(884);
	used_foods.Add(3044);
	used_foods.Add(304);
	used_foods.Add(1649);
	used_foods.Add(3313);
	used_foods.Add(255);
	used_foods.Add(1263);
	used_foods.Add(296);
	used_foods.Add(2255);
	used_foods.Add(31);
	used_foods.Add(225);
	used_foods.Add(1555);
	used_foods.Add(3747);
	used_foods.Add(1310);
	used_foods.Add(306);
	used_foods.Add(1576);
	used_foods.Add(6610);
	used_foods.Add(4848);
	used_foods.Add(4862);
	used_foods.Add(3146);
	used_foods.Add(2259);
	used_foods.Add(3750);
	used_foods.Add(3039);
	used_foods.Add(215);
	used_foods.Add(307);
	used_foods.Add(155);
	used_foods.Add(51);
	used_foods.Add(71);
	used_foods.Add(7773);
	used_foods.Add(2995);
	used_foods.Add(290);
	used_foods.Add(4496);
	used_foods.Add(2665);
	used_foods.Add(1750);
	used_foods.Add(305);
	used_foods.Add(4641);
	used_foods.Add(4259);
	used_foods.Add(2);
	used_foods.Add(302);
	used_foods.Add(69);
	used_foods.Add(2690);
	used_foods.Add(2819);
	used_foods.Add(301);
	used_foods.Add(4483);
	used_foods.Add(2261);
	used_foods.Add(2989);
	used_foods.Add(3092);
	used_foods.Add(274);
	used_foods.Add(140);
	used_foods.Add(3686);
	used_foods.Add(6286);
	used_foods.Add(2196);
	used_foods.Add(3684);
	used_foods.Add(3646);
	used_foods.Add(2889);
	used_foods.Add(2123);
	used_foods.Add(2790);
	used_foods.Add(264);
	used_foods.Add(2891);
	used_foods.Add(270);
	used_foods.Add(6328);
	used_foods.Add(2285);
	used_foods.Add(279);
	used_foods.Add(3586);
	used_foods.Add(252);
	used_foods.Add(3699);
	used_foods.Add(3566);
	used_foods.Add(139);
	used_foods.Add(3045);
	used_foods.Add(3661);
	used_foods.Add(287);
	used_foods.Add(259);
	used_foods.Add(3680);
	used_foods.Add(2288);
	used_foods.Add(267);
	used_foods.Add(1727);
	used_foods.Add(6510);
	used_foods.Add(6543);
	used_foods.Add(260);
	used_foods.Add(273);
	used_foods.Add(676);
	used_foods.Add(678);
	used_foods.Add(717);
	used_foods.Add(719);
	used_foods.Add(724);
	used_foods.Add(726);
	used_foods.Add(728);
	used_foods.Add(729);
	used_foods.Add(737);
	used_foods.Add(793);
	used_foods.Add(819);
	used_foods.Add(1129);
	used_foods.Add(1748);
	used_foods.Add(2582);
	used_foods.Add(3613);
	used_foods.Add(3672);
	used_foods.Add(3703);
	used_foods.Add(4713);
	used_foods.Add(5003);
	used_foods.Add(6157);
	used_foods.Add(7377);
	used_foods.Add(8139);
	used_foods.Add(8386);
	used_foods.Add(4499);
	used_foods.Add(6204);
	used_foods.Add(6389);
	
	
	SortIndex(used_foods, StdLess<int>());
}

int Database::FindFood(String long_desc) const {
	for(int i = 0; i < food_descriptions.GetCount(); i++)
		if (food_descriptions[i].long_desc == long_desc)
			return i;
	return -1;
}

int Database::FindFoodLeft(String long_desc) const {
	for(int i = 0; i < food_descriptions.GetCount(); i++)
		if (food_descriptions[i].long_desc.Left(long_desc.GetCount()) == long_desc)
			return i;
	return -1;
}

int Database::FindNutrition(String key) const {
	for(int i = 0; i < nutr_types.GetCount(); i++)
		if (nutr_types[i].tagname == key)
			return i;
	return -1;
}

int Database::FindNutritionRecommendation(String key) const {
	for(int i = 0; i < nutr_recom.GetCount(); i++)
		if (nutr_types[nutr_recom[i].nutr_no].tagname == key)
			return i;
	return -1;
}

void Database::SetCommonNutrs() {
	PROT = FindNutrition("PROCNT");
	FAT = FindNutrition("FAT");
	CARB = FindNutrition("CHOCDF");
	KCAL = FindNutrition("ENERC_KCAL");
	SODIUM = FindNutrition("NA");
	PUFA = FindNutrition("FAPU");
	ASSERT(PROT >= 0 && FAT >= 0 && CARB >= 0 && KCAL >= 0 && SODIUM >= 0 && PUFA >= 0);
}

void Database::RemoveDuplicates() {
	Index<String> food_long;
	
	for(int i = 0; i < food_descriptions.GetCount(); i++) {
		const FoodDescription& d = food_descriptions[i];
		if (food_long.Find(d.long_desc) >= 0)
			food_descriptions.Remove(i--);
		else
			food_long.Add(d.long_desc);
	}
	
	Index<int> used;
	
	for(int i = 0; i < used_foods.GetCount(); i++) {
		int f_i = used_foods[i];
		if (used.Find(f_i) >= 0)
			used_foods.Remove(i--);
		else
			used.Add(f_i);
	}
}

void Database::GetBestMacroFoods() {
	Index<int> high_fat_foods, high_prot_foods, high_vitamin_foods, other_foods;
	Index<int> fat_nutr, prot_nutr;
	for(int i = 0; i < used_foods.GetCount(); i++) {
		int db_i = used_foods[i];
		const FoodDescription& f = food_descriptions[db_i];
		Ingredient ing;
		ing.Reset();
		for (const auto& n : f.nutr)
			ing.nutr[n.nutr_no] = n.nutr_value;
		
		if (ing.nutr[PROT] >= 80)
			high_prot_foods.Add(i);
		else if (ing.nutr[FAT] >= 80)
			high_fat_foods.Add(i);
		else {
			OnlineAverage vit_av;
			for(const NutritionRecommendation& r: nutr_recom) {
				double value = ing.nutr[r.nutr_no];
				double target = r.GetValue(70);
				ASSERT(target > 0);
				double frac = value / target;
				vit_av.Add(frac);
			}
			if (vit_av.GetMean() > 1.0)
				high_vitamin_foods.Add(i);
			else
				other_foods.Add(i);
		}
	}
	fat_nutr.Add(FAT);
	prot_nutr.Add(PROT);
	
	if (1) {
		for(int i = 0; i < high_fat_foods.GetCount(); i++) {
			const FoodDescription& d = food_descriptions[used_foods[high_fat_foods[i]]];
			LOG("High fat food " << i << ": " << d.long_desc);
		}
		for(int i = 0; i < high_prot_foods.GetCount(); i++) {
			const FoodDescription& d = food_descriptions[used_foods[high_prot_foods[i]]];
			LOG("High protein food " << i << ": " << d.long_desc);
		}
		for(int i = 0; i < high_vitamin_foods.GetCount(); i++) {
			const FoodDescription& d = food_descriptions[used_foods[high_vitamin_foods[i]]];
			LOG("High vitamin food " << i << ": " << d.long_desc);
		}
		for(int i = 0; i < other_foods.GetCount(); i++) {
			const FoodDescription& d = food_descriptions[used_foods[other_foods[i]]];
			LOG("Other food " << i << ": " << d.long_desc);
		}
	}
}
