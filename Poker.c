#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_RANKS 13
#define NUM_SUITS 4
#define NUM_CARDS NUM_RANKS * NUM_SUITS
#define HAND_SIZE 5
#define NUM_HAND_TYPES 10
#define MIN_PLAYERS 1
#define MAX_PLAYERS NUM_CARDS/HAND_SIZE
#define NUM_SHUFFLES 1000

typedef enum {
	bust, pair, twoPair, three, straight, flush, fullHouse, four, straightFlush, royalFlush
} handWorthType;

typedef struct {
	int rank;
	int suit;
} cardType;

typedef struct {
	cardType cards[HAND_SIZE];
	handWorthType type;
	int value; 				// number that represents the value of the cards relevant to the worth of the hand (used for tie breaking)
} handType;

void createDeck (cardType *deck) {
	for (int i = 0; i < NUM_RANKS; i++) {
		for (int j = 0; j < NUM_SUITS; j++) {
			deck[i * NUM_SUITS + j].rank = i;
			deck[i * NUM_SUITS + j].suit = j;
		}
	}
}

void sortCards (cardType *cards, int len) {
	cardType temp;
	
	for (int i = 0; i < len; i++) {
		for (int j = 0; j < len - 1; j++) {
			if (cards[j].rank > cards[j+1].rank) {
				temp = cards[j];
				cards[j] = cards[j+1];
				cards[j+1] = temp;
			}
		}
	}
}

int maxRank (cardType *cards, int len) {
	int curMax = cards[0].rank;
	
	for (int i = 0; i < len; i++) {
		if (cards[i].rank > curMax) {
			curMax = cards[i].rank;
		}
	}
	
	return curMax;
}

// returns 0 if not a straight, otherwise returns the value of the highest card 
int checkStraight (int *ranks, int counted, int consecutive, int started) {
	
	if (counted == NUM_RANKS) {
		return 0;
	}	
	else if (*ranks) {
		if (consecutive + 1 == HAND_SIZE) {
			return counted;
		}
		else {
			return checkStraight (++ranks, counted + 1, consecutive + 1, 1);
		}
	}
	else if (!started) {
		return checkStraight (++ranks, counted + 1, consecutive, 0);
	}
	else {
		return 0;
	}
}

void checkHand (handType *hand) {
	int ranks[NUM_RANKS] = {0};
	int suits[NUM_SUITS] = {0};
	
	for (int i = 0; i < HAND_SIZE; i++) {
		ranks[hand->cards[i].rank]++;
		suits[hand->cards[i].suit]++;
	}	
	
	hand->type = bust;
	hand->value = checkStraight (ranks, 0, 0, 0);
	
	if (hand->value) {
		hand->type = straight;
	}
	else {		// check for pairs, three of a kinds, and four of a kinds
		for (int i = 0; i < NUM_RANKS; i++) {
			switch (ranks[i]) {
				case 2:
					if (hand->type == bust) {
						hand->type = pair;
						hand->value = i;
					}
					else if (hand->type == pair) {
						hand->type = twoPair;
						hand->value = (hand->value > i)? (hand->value * 10 + i) : (i * 10 + hand->value);
					}
					else if (hand->type == three) {
						hand->type = fullHouse;
						hand->value = hand->value * 10 + i;
					}
					break;
		
				case 3:
					if (hand->type == bust) {
						hand->type = three;
						hand->value = i;
					}
					else if (hand->type == pair) {
						hand->type = fullHouse;
						hand->value = i * 10 + hand->value;
					}
					break;
				
				case 4:
					hand->type = four;
					hand->value = i;
					break;
			}
		}
	}
		
	//check for flushes
	for (int i = 0; i < NUM_SUITS; i++) {
		if (suits[i] == HAND_SIZE) {
			
			if (hand->type == straight) {

				if (hand->value == NUM_RANKS - 1) {
					hand->type = royalFlush;
				}
				else {
					hand->type = straightFlush;
				}
			}
			else {
				hand->type = flush;
			}
		}
	}
	
	// calculate value if hand was a bust or a flush
	if (hand->type == bust || hand->type == flush) {
		hand->value = 0;
		sortCards (hand->cards, HAND_SIZE);
		
		for (int i = HAND_SIZE - 1; i >= 0; i--) {
			hand->value = hand->value * 10 + hand->cards[i].rank;
		}
	}
}

// evalALlHands generates and evaluates all possible hand combinations, stores the results in the results array, and then returns the total number of hands (combinations)
int evalAllHandsHelper (cardType *deck, handType *hand, int*results, int place, int numChosen) {
	if (numChosen == HAND_SIZE) {
		checkHand (hand);
		results[hand->type]++;
		
		return 1;
	}
	else {
		int total = 0;

		for (int i = place; i < NUM_CARDS; i++) {
			hand->cards[numChosen] = deck[i];
			total += evalAllHandsHelper (deck, hand, results, i + 1, numChosen + 1);
		}

		return total;
	}
}

int evalAllHands (int *results) {
	cardType deck[NUM_CARDS];
	handType hand;
	createDeck (deck);
	
	return evalAllHandsHelper (deck, &hand, results, 0, 0);
}

void findProb (FILE *f) {
	int results [NUM_HAND_TYPES] = {0};
	int total = evalAllHands (results);
	char *type[] = {"Bust", "Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush", "Royal Flush"};
	
	fprintf (f, "    PROBABILITIES OF EACH 5-CARD POKER HAND\n\n");
	fprintf (f, "%15s  %10s  %15s\n\n", "Hand Type", "Number", "Probability");
	for (int i = 0; i < NUM_HAND_TYPES; i++) {
		fprintf (f, "%15s : %10i %15f%% \n", type[i], results[i], (float)results[i]/total*100);
	}	
	
}

int rb (int min, int max) {
	return rand() % (max - min + 1) + min;
}

void shuffleDeck (cardType *deck) {
	cardType temp;
	int i1, i2; 
	
	for (int i = 0; i < NUM_SHUFFLES; i++) {
		i1 = rb (0, NUM_CARDS - 1);
		i2 = rb (0, NUM_CARDS - 1);
		
		temp = deck[i1];
		deck[i1] = deck[i2];
		deck[i2] = temp; 
	}
	
}



void simGame (FILE *f, int numPlayers, int numGames) {
	cardType deck[NUM_CARDS];
	handType players[numPlayers];
	createDeck(deck);
	
	int winners[numPlayers]; 
	int numWinners; 
	char *type[] = {"Bust", "Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush", "Royal Flush"};
	char suitSym [] = {'H', 'D', 'S', 'C'};
	
	for (int i = 0; i < numGames; i++) {
		shuffleDeck (deck);
		numWinners = 0;
		
		fprintf (f, "GAME %i", i + 1);
		
		for (int j = 0; j < numPlayers; j++) {
			
			fprintf (f, "\nPlayer %i's hand: ", j + 1);	
			
			for (int k = 0; k < HAND_SIZE; k++) {
				players[j].cards[k] = deck[j * HAND_SIZE + k];
				
				fprintf (f, "%i%c ", players[j].cards[k].rank + 1, suitSym[players[j].cards[k].suit]);
			}
			checkHand (&players[j]);		
			fprintf (f, "(%s)", type[players[j].type]);
			
			
			if (j == 0) {
				winners[0] = j;
				numWinners = 1;
			}			
			else if (players[j].type > players[winners[numWinners - 1]].type || 
			(players[j].type == players[winners[numWinners - 1]].type &&  players[j].value > players [winners[numWinners - 1]].value)) {
				winners[0] = j;		
				numWinners = 1;				
			}
			else if (players[j].type == players [winners[numWinners - 1]].type && players[j].value == players [winners[numWinners - 1]].value) {
				winners [numWinners++] = j;
			}
						
		}
		
		fprintf (f, "\nThe %s ", (numWinners == 1)? "winner is" : "winners are");
		for (int j = 0; j < numWinners; j++) {
			fprintf (f, "%splayer %i", j == 0? "" : " and ",  winners[j] + 1);
		}
		fprintf (f, "!\n\n");	
		
	}
}

int main () {	
	int numPlayers, numGames;
	FILE *fprob = fopen ("PokerHandProbabilities.txt", "w");
	FILE *fgame = fopen ("PokerGameResults.txt", "w");
	
	srand ((unsigned) time (NULL));
	
	findProb (fprob);
	
	printf ("Welcome to 5-Card Poker Simulation!\n");
	printf ("For hand probabilites, see \"PokerHandProbabilities.txt\"\n\n");
	printf ("How many players would you like to simulate (between %i and %i)?\n", MIN_PLAYERS, MAX_PLAYERS);
	
	do {
		scanf ("%i", &numPlayers);
	}	while ((numPlayers > MAX_PLAYERS || numPlayers < MIN_PLAYERS) && printf ("Invalid number! Please try another. \n"));
	
	printf ("How many games would you like to simulate?\n");
	scanf ("%i", &numGames);
	
	simGame (fgame, numPlayers, numGames);
	
	printf ("\nYour games have been simulated! Results can be found in \"PokerGameResults.txt\"");
	
	return 0;
}
