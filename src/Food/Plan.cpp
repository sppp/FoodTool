#include "Food.h"










void GetIngredientNutritions(Ingredient& dst, const Vector<MealIngredient>& ingredients) {
	const Database& db = DB();
	dst.Reset();
	for(int i = 0; i < ingredients.GetCount(); i++) {
		const MealIngredient& src = ingredients[i];
		const FoodDescription& d = db.food_descriptions[src.db_food_no];
		if (!src.max_grams)
			continue;
		dst.grams += src.max_grams;
		double mul = src.max_grams / 100.0;
		for(const NutritionInfo& info : d.nutr)
			dst.nutr[info.nutr_no] += info.nutr_value * mul;
	}
}

void MealPreset::GetNutritions(Ingredient& dst) const {
	GetIngredientNutritions(dst, ingredients);
}

void MealPresetVariant::GetNutritions(Ingredient& dst) const {
	GetIngredientNutritions(dst, ingredients);
}

void GetFactors(Vector<MealIngredient>& ingredients, double& mass_factor, double& taste_factor, double& score) {
	const Database& db = DB();
	mass_factor = 0;
	taste_factor = 0;
	
	Ingredient ing;
	double av_kcals = 0, av_carb = 0, av_fat = 0, av_prot = 0, av_mass = 0;
	for(int i = 0; i < ingredients.GetCount(); i++) {
		const MealIngredient& in = ingredients[i];
		const FoodDescription& d = db.food_descriptions[in.db_food_no];
		double mass = (in.max_grams + in.min_grams) / 2.0;
		ing.Set(mass, d);
		av_kcals += ing.nutr[KCAL];
		av_carb += ing.nutr[CARB];
		av_fat += ing.nutr[FAT];
		av_prot += ing.nutr[PROT];
		av_mass += mass;
	}
	mass_factor = av_mass / av_kcals;
	
	av_carb = av_carb / av_mass;
	av_fat  = av_fat  / av_mass * 9.0;
	av_prot = av_prot / av_mass * 4.0;
	double av_macro = (av_carb + av_fat + av_prot) / 3.0;
	double macro_imbalance = fabs(av_carb / av_macro -1) + fabs(av_fat / av_macro -1) + fabs(av_prot / av_macro -1);
	
	double balance_score = 10.0 - max(0.0, min(10.0, macro_imbalance));
	taste_factor = balance_score * av_kcals / av_mass;
	
	score = mass_factor * taste_factor;
}

void MealPreset::UpdateFactors() {
	GetFactors(ingredients, mass_factor, taste_factor, score);
}

void MealPresetVariant::UpdateFactors() {
	GetFactors(ingredients, mass_factor, taste_factor, score);
}


/*
Variant ideas:
 - largest / smallest / average (= (largest + smallest) / 2)
	- mass
	- taste
	- score (mass * taste)
	- score without carbs
	- score without fat
	- score without protein
	- protein
	- fat
	- carbs
	- nutrient-score
	- fiber
	- essential amino acids
	- fatty acids
 - 1:5, 1:2, 1:1, 2:1, 5:1
	- largest mass vs best taste
	- largest protein vs largest fatty acids
 - most useful
	- easiest
		- best score, but punish by every new ingredient
	- for weight loss
		- largest mass with punishment for going lower than minimum taste factor
	- for muscle gain (2000kcal/day)
		- minimum carbs, 2.2g/weight-kg protein, essential amino acids, fatty acids (, and best score)
	- for maintenance (1800kcal/pv)
		- minimum carbs, 0.8g/weight-kg protein, minimum essential amino acids, maximum fatty acids
*/

template <int I>
double VariantScore(const MealPresetVariant& var) {
	thread_local static Ingredient ing;
	const Database& db = DB();
	
	ing.Reset();
	
	int ing_count = 0;
	for(const MealIngredient& mi : var.ingredients) {
		const FoodDescription& d = db.food_descriptions[mi.db_food_no];
		if (mi.max_grams > 0.0) {
			ing.Add(mi.max_grams, d);
			ing_count++;
		}
	}
	
	if (ing.grams == 0.0)
		return -10000000;
	
	double av_mass = ing.grams;
	double av_kcals = ing.nutr[KCAL];
	double mass_factor = av_mass / av_kcals;
	
	double av_carb = ing.nutr[CARB] / av_mass;
	double av_fat  = ing.nutr[FAT]  / av_mass * 9.0;
	double av_prot = ing.nutr[PROT] / av_mass * 4.0;
	double av_macro = (av_carb + av_fat + av_prot) / 3.0;
	double macro_imbalance = fabs(av_carb / av_macro -1) + fabs(av_fat / av_macro -1) + fabs(av_prot / av_macro -1);
	
	double balance_score = 10.0 - max(0.0, min(10.0, macro_imbalance));
	double taste_factor = balance_score * av_kcals / av_mass;
	
	double score = mass_factor * taste_factor;
	if (I > 0) {
		score /= I + ing_count;
	}
	else if (I == -1) {
		score = mass_factor;
		if (taste_factor < 5)
			score -= fabs(5 - taste_factor) / 2.0;
	}
	
	return score;
}

template <int I>
double VariantNutritionGroup(const MealPresetVariant& var) {
	thread_local static Ingredient ing;
	const Database& db = DB();
	
	ing.Reset();
	
	for(const MealIngredient& mi : var.ingredients) {
		const FoodDescription& d = db.food_descriptions[mi.db_food_no];
		ing.Add(mi.max_grams, d);
	}
	
	if (ing.grams == 0.0)
		return -10000000;
	
	double new_grams = 2000.0 / ing.nutr[KCAL] * ing.grams;
	ing.ChangeGrams(new_grams);
	
	double score = 0;
	for(const NutritionRecommendation& recom : db.nutr_recom) {
		if (recom.group == I) {
			double value = ing.nutr[recom.nutr_no];
			double target = recom.GetValue(100);
			double rel = value / target - 1.0;
			if (rel < 0)
				score += rel;
		}
	}
	
	return score;
}

template <int I, int J>
double VariantMaintenance(const MealPresetVariant& var) {
	thread_local static Ingredient ing;
	const Database& db = DB();
	
	ing.Reset();
	
	for(const MealIngredient& mi : var.ingredients) {
		const FoodDescription& d = db.food_descriptions[mi.db_food_no];
		ing.Add(mi.max_grams, d);
	}
	
	if (ing.grams == 0.0)
		return -10000000;
	
	double new_grams = (double)I / ing.nutr[KCAL] * ing.grams;
	ing.ChangeGrams(new_grams);
	
	double score = 0;
	for(const NutritionRecommendation& recom : db.nutr_recom) {
		if (recom.group == AMINOACID || recom.group == FATTYACID) {
			double value = ing.nutr[recom.nutr_no];
			double target = recom.GetValue(100);
			double rel = value / target - 1.0;
			if (rel < 0)
				score += rel;
		}
	}
	
	{
		double value = ing.nutr[CARB];
		double target = I * 0.2 / 4.0;
		double rel = value / target - 1.0;
		score -= fabs(rel) * 10;
	}
	
	{
		double value = ing.nutr[PROT];
		double target = J * 0.1 * 70; // TODO: use current weight
		double rel = value / target - 1.0;
		score -= fabs(rel) * 10;
	}
	
	return score;
}


void MealPreset::MakeVariants() {
	/*
	 - score
	 - easiest
	 - essential amino acids
	 - fatty acids
	 - weight loss
	 - muscle gain
	 - maintenance
	*/
	const Database& db = DB();
	
	typedef double (*VariantFn)(const MealPresetVariant& var);
	
	VariantFn eval;
	
	variants.SetCount(7);
	for(int i = 0; i < 7; i++) {
		MealPresetVariant& var = variants[i];
		
		var.ingredients <<= ingredients;
		
		double kcal_tgt = 1800;
		switch (i) {
			case VARIANT_SCORE:       eval = &VariantScore<0>; break;
			case VARIANT_EASIEST:     eval = &VariantScore<5>; break;
			case VARIANT_AMINOACIDS:  eval = &VariantNutritionGroup<AMINOACID>; break;
			case VARIANT_FATTYACIDS:  eval = &VariantNutritionGroup<FATTYACID>; break;
			case VARIANT_WEIGHTLOSS:  eval = &VariantScore<-1>; kcal_tgt = 600; break;
			case VARIANT_MUSCLEGAIN:  eval = &VariantMaintenance<2000, 8>; kcal_tgt = 2000; break;
			case VARIANT_MAINTENANCE: eval = &VariantMaintenance<1800, 22>; break;
			default: Panic("Invalid variant code");
		}
		
		Optimizer opt;
		opt.Min().SetCount(var.ingredients.GetCount());
		opt.Max().SetCount(var.ingredients.GetCount());
		for(int i = 0; i < var.ingredients.GetCount(); i++) {
			const MealIngredient& mi = var.ingredients[i];
			opt.Min()[i] = mi.min_grams;
			opt.Max()[i] = mi.max_grams;
		}
		opt.SetMaxGenerations(100);
		opt.Init(var.ingredients.GetCount(), 100);
		
		while (!opt.IsEnd()) {
			opt.Start();
		
			const Vector<double>& trial = opt.GetTrialSolution();
			for(int i = 0; i < var.ingredients.GetCount(); i++) {
				const MealIngredient& main = ingredients[i];
				MealIngredient& ing = var.ingredients[i];
				double grams = max(main.min_grams, min(main.max_grams, trial[i]));
				ing.min_grams = grams;
				ing.max_grams = grams;
			}
			
			MealDebugger dbg;
			double energy = eval(var);
			
			//LOG(opt.GetRound() << ": " << energy);
			
			opt.Stop(energy);
		}
		
		Ingredient total;
		const Vector<double>& best = opt.GetBestSolution();
		for(int i = 0; i < var.ingredients.GetCount(); i++) {
			const MealIngredient& main = ingredients[i];
			MealIngredient& ing = var.ingredients[i];
			double grams = max(main.min_grams, min(main.max_grams, best[i]));
			ing.min_grams = grams;
			ing.max_grams = grams;
			const FoodDescription& d = db.food_descriptions[main.db_food_no];
			if (grams > 0)
				total.Add(grams, d);
		}
		double kcal_total = total.nutr[KCAL];
		double mul = kcal_tgt / kcal_total;
		for(auto& mi : var.ingredients) {
			mi.min_grams *= mul;
			mi.max_grams *= mul;
		}
		
		var.UpdateFactors();
	}
	
}

int MealPreset::FindIngredient(int food_no) const {
	for(int i = 0; i < ingredients.GetCount(); i++)
		if (ingredients[i].db_food_no == food_no)
			return i;
	return -1;
}
