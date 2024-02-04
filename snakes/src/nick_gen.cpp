#include "nick_gen.h"
#include <random>

static const char* NAME1[] = {
	"Pihu", "Dearie", "Golu", "Sunny", "Kanha", "Raj", "Guddu", "Raji","Gudiya", "Rishu",
	"Misht", "Rosy", "Shree", "June", "Piya", "Ria", "Adi", "Riva", "Kuhu", "Cashew",
	"Pooh", "Benu", "Brishti", "Betoo", "Nutz", "Oshu", "Shinoo", "Rick", "Chintu", "Vini",
	"Chimpu", "Luv", "Piku", "Rini", "Pappu", "Tuli", "Chikki", "Joy", "Raja", "Aashi",
	"Rani", "Bao", "Dimple", "Cuckoo", "Dimpy", "Gilli", "Proshu", "Echo", "Prishu", "Fifi",
	"Chinu", "Lala", "Babloo", "Ricky", "Babli", "Shoona", "Akki", "Tolu", "Ace", "Chubs",
	"Alex", "Duchess", "Bean", "Duke", "Blaze", "Gem", "Boss", "Gus", "Bud", "Lion", "Cub",
	"Captain", "Moonpie", "Champ", "Queenie", "Cheeks", "Rockstar", "Chris", "Ted"
};
static const char* NAME2[] = {
	"Ky", "La", "Laf", "Lam", "Lue", "Ly", "Mai", "Mal", "Mara", "My", "Na", "Nai", "Nim",
	"Nu", "Ny", "Py", "Raer", "Re", "Ren", "Rhy", "Ru", "Rua", "Rum", "Rid", "Sae", "Seh",
	"Sel", "Sha", "She", "Si", "Sim", "Sol", "Sum", "Syl", "Ta", "Tahl", "Tha", "Tho", "Ther",
	"Thro", "Tia", "Tra", "Ty", "Uth", "Ver", "Vil", "Von", "Ya", "Za", "Zy"
};

extern wxString snakes::GenerateNickName()
{
	std::random_device rd;
	std::uniform_int_distribution<int> distrib1(0, sizeof(NAME1) / sizeof(char*) -1);
	std::uniform_int_distribution<int> distrib2(0, sizeof(NAME2) / sizeof(char*) -1);
	std::uniform_int_distribution<int> distrib3(0, 9);
	return wxString() << NAME2[distrib2(rd)] << wxT("-") << NAME1[distrib1(rd)] << wxT("-") << distrib3(rd);
}
