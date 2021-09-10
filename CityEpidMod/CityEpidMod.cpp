// CityEpidMod.cpp : Defines the entry point for the application.
//

#include "CityEpidMod.h"

const unsigned int PEOPLE_AMOUNT = 3500;

const float GO_WORK = 0.714285f;
const float GO_ENTERTAINMENT = 0.437659f;

const float WORKING[3] = { 0.1f, 0.7f, 1.0f };			// go work			at living = 10%,	at working = 60%,	at entertainment = 30%
const float ENTERTAINMENT[3] = { 0.2f, 0.2f, 1.0f };	// go entertainment	at living = 20%,	at working = 0%,	at entertainment = 80%

const float MEET_AT_LIVE = 0.001f;
const float MEET_AT_WORK = 0.005f;
const float MEET_AT_ENT = 0.01f;

const unsigned int TIME = 30;

const float INFECT_PART = 0.05f;
const float EXPOSED_PART = 0.1f;

const float CATCH_INFECT = 0.0558f;

struct PERSON
{
	unsigned int state = 0;			// 0 - healthy, 1 - exposed, 2 - infectious, 3 - removed
	bool isUpdate = false;
	float chance = 0;				// chance to go to next stage
	unsigned int whereWork = 0;		// index of working place
	unsigned int typeWork = 0;		// 0 - at living, 1 - at working, 2 - at entertainment
	unsigned int whereLive = 0;		// index of living place

	unsigned int goToWork = 0;		
	unsigned int goToEntertainment = 0;
	unsigned int typeEnt = 0;		// 0 - at living, 1 - at working, 2 - at entertainment
	unsigned int whereEnt = 0;
};

struct PLACE
{
	float weight;
	std::vector<int> livingPeople;
	std::vector<int> workingPeople;
	std::vector<int> entingPeople;

	PLACE(float w = 0) : weight(w), livingPeople(), workingPeople(), entingPeople() {};
};

int findType(const float* whereToLook, float chance) {
	for (unsigned int i = 0; i < 3; i++) { if (whereToLook[i] > chance) { return(i); } }
	return(2);
}

int findNumber(std::vector<PLACE> *whereToLook, float chance) {
	for (unsigned int i = 0; i < whereToLook->size(); i++) { if (whereToLook->at(i).weight > chance) { return(i); } }
	return(whereToLook->size() - 1);
}

int main()
{
	// random generators
	std::mt19937 gen(time(0));
	std::uniform_real_distribution<float> randFloat(0, 1);
	std::weibull_distribution<float> randRemoved(2.5f, 16.5f);
	std::weibull_distribution<float> randInf(1.24f, 5.376f);



	// creating living spaces
	std::vector<PLACE> living = std::vector<PLACE>();
	living.push_back(PLACE(10));	// 10% to live in [0]
	living.push_back(PLACE(10));	// 10% to live in [1]
	living.push_back(PLACE(25));	// 25% to live in [2]
	living.push_back(PLACE(25));	// 25% to live in [3]
	living.push_back(PLACE(30));	// 30% to live in [4]

	float totalCh = 0.0f;
	float cumulCh = 0.0f;
	for (unsigned int i = 0; i < living.size(); i++) { totalCh += living[i].weight; }
	for (unsigned int i = 0; i < living.size(); i++) { 
		cumulCh += living[i].weight / totalCh;
		living[i].weight = cumulCh;
	}

	// creating working spaces
	std::vector<PLACE> working = std::vector<PLACE>();
	working.push_back(PLACE(1));	// 100% to work in [0]

	totalCh = 0.0f;
	cumulCh = 0.0f;
	for (unsigned int i = 0; i < working.size(); i++) { totalCh += working[i].weight; }
	for (unsigned int i = 0; i < working.size(); i++) {
		cumulCh += working[i].weight / totalCh;
		working[i].weight = cumulCh;
	}


	// creating entertainment spaces
	std::vector<PLACE> enting = std::vector<PLACE>();
	enting.push_back(PLACE(1));	// 100% to entertainment in [0]

	totalCh = 0.0f;
	cumulCh = 0.0f;
	for (unsigned int i = 0; i < enting.size(); i++) { totalCh += enting[i].weight; }
	for (unsigned int i = 0; i < enting.size(); i++) {
		cumulCh += enting[i].weight / totalCh;
		enting[i].weight = cumulCh;
	}

	std::uniform_int_distribution<int> randEnt(0, enting.size()-1);



	// creating people
	int exp_amount = static_cast<int>(PEOPLE_AMOUNT * EXPOSED_PART);
	int inf_amount = static_cast<int>(PEOPLE_AMOUNT * INFECT_PART);

	std::vector<PERSON> people = std::vector<PERSON>();

	for (unsigned int z = 0; z < PEOPLE_AMOUNT; z++) {
		PERSON toAdd = PERSON();

		// defining state
		toAdd.chance = randFloat(gen);
		if (z < exp_amount + inf_amount) {	// exposed
			toAdd.state++;
			toAdd.chance = randInf(gen);
		}
		if (z < inf_amount) {				// infectious
			toAdd.state++;
			toAdd.chance = randRemoved(gen);
		}

		// add living place		
		toAdd.whereLive = findNumber(&living, randFloat(gen));
		living[toAdd.whereLive].livingPeople.push_back(z);


		// add working place
		switch (findType(WORKING, randFloat(gen)))
		{
		case 0:		// work at living
			toAdd.typeWork = 0;
			toAdd.whereWork = findNumber(&living, randFloat(gen));
			living[toAdd.whereWork].workingPeople.push_back(z);
			break;
		case 1:		// work at working
			toAdd.typeWork = 1;
			toAdd.whereWork = findNumber(&working, randFloat(gen));
			working[toAdd.whereWork].workingPeople.push_back(z);
			break;
		default:	// work at entertainment
			toAdd.typeWork = 2;
			toAdd.whereWork = findNumber(&enting, randFloat(gen));
			enting[toAdd.whereWork].workingPeople.push_back(z);
			break;
		}


		people.push_back(toAdd);
	}

	// print states
	int s0 = 0;
	int s1 = 0;
	int s2 = 0;
	int s3 = 0;
	for (size_t i = 0; i < PEOPLE_AMOUNT; i++)
	{
		if (people[i].state == 0) { s0++; }
		if (people[i].state == 1) { s1++; }
		if (people[i].state == 2) { s2++; }
		if (people[i].state == 3) { s3++; }
	}
	printf("%i : %i %i %i %i\n", 0, s0, s1, s2, s3);


	// updating states
	for (unsigned int t = 0; t < TIME; t++) {

		// chances to go to places
		for (unsigned int personId = 0; personId < PEOPLE_AMOUNT; personId++) {

			// work
			if (randFloat(gen) < GO_WORK) { people[personId].goToWork = 1; }

			// entertainment
			if (randFloat(gen) < GO_ENTERTAINMENT) {
				people[personId].goToEntertainment = 1;

				// where to go
				switch (findType(ENTERTAINMENT, randFloat(gen)))
				{
				case 0:		// entertainment at living
					people[personId].typeEnt = 0;
					people[personId].whereEnt = findNumber(&living, randFloat(gen));
					living[people[personId].whereLive].entingPeople.push_back(personId);
					break;
				case 1:		// entertainment at working
					people[personId].typeEnt = 1;
					people[personId].whereEnt = findNumber(&working, randFloat(gen));
					working[people[personId].whereWork].entingPeople.push_back(personId);
					break;
				default:	// entertainment at entertainment
					people[personId].typeEnt = 2;
					people[personId].whereEnt = findNumber(&enting, randFloat(gen));
					enting[people[personId].whereEnt].entingPeople.push_back(personId);
					break;
				}
			}
		}

		// calculating if need to update
		for (unsigned int personId = 0; personId < PEOPLE_AMOUNT; personId++) {

			// if already infected
			if (people[personId].state == 1 || people[personId].state == 2) {
				people[personId].chance--;
				if (people[personId].chance < 0) { people[personId].isUpdate = true; }
			}
			if (people[personId].state == 0) {
				
				// check from live
				int contactsAmount = 0;
				PLACE *visitPlace = &(living[people[personId].whereLive]);
				for (unsigned int i = 0; i < visitPlace->livingPeople.size(); i++) {
					if (
						(MEET_AT_LIVE > randFloat(gen)) &&					// they met
						people[visitPlace->livingPeople[i]].state == 2		// contact is infectious
					) { contactsAmount++; }
				}
				if (people[personId].chance > pow((1 - 0.0558f), contactsAmount)) { people[personId].isUpdate = true; }
				
				// check from ent
				if (!people[personId].isUpdate && people[personId].goToEntertainment == 1) {
					contactsAmount = 0;
					switch (people[personId].typeEnt)
					{
					case 0:
						visitPlace = &(living[people[personId].whereEnt]);
						break;
					case 1:
						visitPlace = &(working[people[personId].whereEnt]);
						break;
					default:
						visitPlace = &(enting[people[personId].whereEnt]);
						break;
					}
					for (unsigned int i = 0; i < visitPlace->entingPeople.size(); i++) {
						if (
							(MEET_AT_ENT > randFloat(gen)) &&					// they met
							people[visitPlace->entingPeople[i]].state == 2		// contact is infectious
						) { contactsAmount++; }
					}
					if (people[personId].chance > pow((1 - 0.0558f), contactsAmount)) { people[personId].isUpdate = true; }
				}

				// check from work
				if (!people[personId].isUpdate && people[personId].goToWork == 1) {
					contactsAmount = 0;
					switch (people[personId].typeWork)
					{
					case 0:
						visitPlace = &(living[people[personId].whereWork]);
						break;
					case 1:
						visitPlace = &(working[people[personId].whereWork]);
						break;
					default:
						visitPlace = &(enting[people[personId].whereWork]);
						break;
					}
					for (unsigned int i = 0; i < visitPlace->entingPeople.size(); i++) {
						if (
							(MEET_AT_WORK > randFloat(gen)) &&					// they met
							people[visitPlace->entingPeople[i]].state == 2 &&	// contact is infectious
							people[visitPlace->entingPeople[i]].goToWork == 1	// other person went to work
						) { contactsAmount++; }
					}
					if (people[personId].chance > pow((1 - 0.0558f), contactsAmount)) { people[personId].isUpdate = true; }
				}
			}
		}

		// updating states and resetting values
		for (unsigned int personId = 0; personId < PEOPLE_AMOUNT; personId++) {
			if (people[personId].isUpdate) {
				if (people[personId].state == 0) { people[personId].chance = randInf(gen); }
				if (people[personId].state == 1) { people[personId].chance = randRemoved(gen); }
				people[personId].state++;
				people[personId].isUpdate = false;
			}
			if (people[personId].state == 0) { people[personId].chance = randFloat(gen); }
			people[personId].goToWork = 0;
			people[personId].goToEntertainment = 0;
			people[personId].typeEnt = 0;
			people[personId].whereEnt = 0;
		}


		// print states
		s0 = 0;		
		s1 = 0;
		s2 = 0;
		s3 = 0;
		for (size_t i = 0; i < PEOPLE_AMOUNT; i++)
		{
			if (people[i].state == 0) { s0++; }
			if (people[i].state == 1) { s1++; }
			if (people[i].state == 2) { s2++; }
			if (people[i].state == 3) { s3++; }
		}
		printf("%i : %i %i %i %i\n", t+1, s0, s1, s2, s3);
	}

	return 0;
}
