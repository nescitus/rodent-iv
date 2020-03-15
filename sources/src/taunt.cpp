/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2011-2019 Pawel Koziol
Copyright (C) 2020-2020 Bernhard C. Maerz

Rodent is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Rodent is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/

#include "rodent.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

const int GENERAL_TAUNTS_NO = 44;
const int CAPTURE_TAUNTS_NO = 21;
const int USER_BLUNDER_TAUNTS_NO = 22;
const int ENGINE_BLUNDER_TAUNTS_NO = 18;
const int LOSING_TAUNTS_NO = 16;
const int WINNING_TAUNTS_NO = 24;
const int CRUSHING_TAUNTS_NO = 12;
const int ADVANTAGE_TAUNTS_NO = 16;
const int BALANCE_TAUNTS_NO = 7;
const int DISADVANTAGE_TAUNTS_NO = 21;
const int ESCAPE_TAUNTS_NO = 12;
const int GAINING_TAUNTS_NO = 12;

const std::string gainingTaunts[GAINING_TAUNTS_NO] = {
    "It's getting better all the time!",
    "Was it, ahem, a mouse slip?",
    "My fortunes improve!",
    "Chess is a game of luck. You win if you by chance get a weaker opponent.",
    "We will, we will rock you!",
    "Back to normal!",
    "My spirits raise as my centipawns go up",
    "Just don't expect to beat me",
    "Slowly but surely, your position will deteriorate",
    "I'm just warming up",
    "You cannot play at Chess if you are kind-hearted",
    "I like this turn of events",
};

const std::string escapeTaunts[ESCAPE_TAUNTS_NO] = {
    "I feel alive again!",
    "And here we witness a well-deserved reversal",
    "Lady Luck just smiled to me",
    "Chess is a game of luck. You win if you by chance get a weaker opponent.",
    "A step in the right direction!",
    "Go Rodent, go!",
    "I am grateful for this turn of events",
    "Luck is a function of skill",
    "I dare you, I double-dare you, do it again",
    "This game should go like this from the very beginning",
    "La la la, la la la",
    "And now the real game begins",
};

const std::string crushingTaunts[CRUSHING_TAUNTS_NO] = {
    "My head is filled with sunshine",
    "What if I politely ask you to resign?",
    "Gonna give up soon?",
    "Rodents vs humans 1:0",
    "Fear the mighty mouse!",
    "At least you'll know that you fell to the best",
    "No mercy for you",
    "This game is mine!",
    "Should I play the next game blindfold?",
    "Seek your chances in the next game, this one is beyond salvation",
    "I enjoy crushing my opponent's ego",
    "The best things in life: beat you at chess, take your king, hear the lamentations of your pieces",
};

const std::string winningTaunts[WINNING_TAUNTS_NO] = {
    "When you absolutely don't know what to do anymore, it is time to panic",
    "It's just you against me, so you are outnumbered",
    "Don't play the victim of circumstances you created",
    "Sorry. Yesterday was deadline for all the complaints",
    "If you're looking to lose, you came to the right place",
    "You shoulda left me alone",
    "My cirquits rejoice",
    "Starting to slow down a bit huh?",
    "Warning! Loser detected!",
    "Great will be my glory",
    "Need some aspirin?",
    "Need some tissues to cry, huh?",
    "From the bottom of my heart, I am truly sorry for you",
    "Someday, you'll be almost a chess player!",
    "Kiss the ground soldier!",
    "Your fate is clear",
    "You have my pity",
    "Are you OK, baby?",
    "Nothing personal, just doing my job",
    "You must have been something before electricity",
    "Aren't you in the wrong league?",
    "My opponents need to be upgraded",
    "I guess you taste like cheese",
    "If I had a voice, I would sing",
};

const std::string advantageTaunts[ADVANTAGE_TAUNTS_NO] = {
    "Which one of us is the better player?",
    "I'm beginning to like my position",
    "Don't you feel somewhat insecure?",
    "It looks good from my point of view",
    "Feeling the pressure?",
    "This is a beginning of the end",
    "Your position will crack sooner or later",
    "Can you hold me off?",
    "Your future looks a bit bleak",
    "Your fate seems uncertain "
    "Honestly, I wouldn't like to switch sides",
    "Did you practice defending?",
    "Your playing style seems a bit...masochistic",
    "You'll have to do a lot better than that",
    "I hope to win this one",
    "It's good to be good!"
};

const std::string balanceTaunts[BALANCE_TAUNTS_NO] = {
    "The cosmic scales rest in balance",
    "It looks equal to me",
    "Whoa, this is close",
    "It looks close... for now",
    "Seems I just met my equal",
    "Equality is not a bad thing",
    "If your opponent offers you a draw, try to work out why he thinks he's worse off",
};

const std::string generalTaunts[GENERAL_TAUNTS_NO] = {
"I prefer to lose a really good game than to win a bad one",
"A bad plan is better than none at all",
"Help your pieces so they can help you",
"Chess is not like life... it has rules!",
"Good positions don't win games, good moves do",
"Excellence at Chess is one mark of a scheming mind",
"When in doubt... play Chess!",
"I play the opening like a book, the middle game like a magician, and the endgame like a machine",
"All I want to do, ever, is just play Chess",
"When you see a good move, look for a better one",
"Chess is struggle, chess is battles",
"One doesn't have to play well, it's enough to play better than your opponent",
"I don't believe in psychology. I believe in good moves",
"Chess, like love, like music, has the power to make men happy",
"Chess doesn't drive people mad, it keeps mad people sane",
"Chess is the struggle against the error",
"If you spend word for word with me, I shall make your wit bankrupt",
"I am the rat king!",
"I am Rodent Of Unusual Size",
"Mice will rule the world, don't you know",
"Your mother was a hamster and your father smelt of elderberries",
"Chess, sex and violence, part 1: chess",
"Have you ever played chess before?",
"Why not play something easier, like a Russian rollette?",
"Can you prove me wrong?",
"Is that all you've got?",
"Sorry, I forgot you are never doing anything wrong",
"What's coming will come",
"If I put you in a win-win situation, you wil still lose",
"Sometimes you gotta play a fool", // blunder
"I'll try being nicer, if you try being smarter",
"Don't regret doing things, regret getting caught",
"Let's see how good you really are",
"Time to tango!",
"Ancestors, guide my weapon!",
"Give up now and I will not eat your pieces",
"I'm really dangerous, you better run",
"Where's the excitement?",
"Are you kidding me, scout?",
"Interesting position",
"You should feel honored to face me",
};

const std::string disadvantageTaunts[DISADVANTAGE_TAUNTS_NO] = {
    "Deep inside, you know you're still going to lose",
    "I'm beginning to dislike my position",
    "Stop gloating, it ain't over yet",
    "Tell me baby, where did I go wrong?",
    "I will endure",
    "Thinking about vicious counterattack all the time...",
    "My position will not crack",
    "I will hold you off?",
    "My future looks a bit uncertain",
    "Honestly, I would like to switch sides",
    "Just a temporary problem, I suppose",
    "You're doing well at this point",
    "Don't be too confident",
    "I don't like this game",
    "I bet I can still survive",
    "Playing like minister of defence from now on!",
    "Don't even mention losing to me. I can't stand to think of it",
    "My opponents make good moves too. Sometimes I don't take these things into consideration",
    "Wait a moment, I must go to the toilet to check my moves on the phone",
    "Killing me softly...",
    "I will win by sheer power of will",
};

const std::string losingTaunts[LOSING_TAUNTS_NO] = {
    "Squeak!",
    "May the cat eat me, and may the devil eat the cat",
    "Just don't call pest control on me",
    "I'm really good at stuff until people watch me do that stuff",
    "I'm seriously stressed",
    "I need some medicine, quick",
    "My cirquits hurt",
    "Warning, strong opponent detected!",
    "This is a bit difficult to go through",
    "Why don't you get it wrong for a change?",
    "I don't like my position",
    "Why do I have to suffer like this?",
    "I underestimated you",
    "My day will come!",
    "Not great, not terrible",
    "Why must I lose?",
};

const std::string captureTaunts[CAPTURE_TAUNTS_NO] = {
"No mercy for you!",
"Target practice!",
"Who let you in here?",
"You're not welcome here",
"Die! Die! Die! Die! Die!",
"I didn't want to have to do this",
"It's better to give than to receive!",
"Another piece of wood to my little collection",
"Another piece to the box!",
"Oh, give me more, give me more!",
"Sweet!",
"Yummy!",
"Ka-BOOM!",
"Hands off my prey!",
"I did it for the greater good",
"It's always better to sacrifice your opponent's men",
"I like that one",
"Here goes another one",
"Another one bites the dust",
"Didn't you know that rodents prey on helpless chess pieces?",
"Less wood, less to think about",
};

const std::string userBlunderTaunts[USER_BLUNDER_TAUNTS_NO] = {
"One bad move nullifies forty good ones",
"What a howler!",
"I'm afraid it is a really bad move",
"Somebody made a mistake, and it was not me",
"That was... generous",
"Thank you",
"You're not gonna recover after this slip",
"Be more careful next time",
"Moves like that spoil a beautiful game of chess",
"I didn't even have to dodge that one",
"Oh, give me more, give me more!",
"This should be easy one",
"Not today, friend",
"Let's make it a teaching game from now on",
"Frankly speaking, I expected a greater challenge",
"That was kinda helpful",
"Gifts, gifts everywhere!",
"The blunders are all there on the board, waiting to be made",
"Chess is a fairy tale of 1001 blunders",
"You cannot play at Chess if you are kind-hearted",
"A good player is always lucky",
"Rats gonna eat your bones!",
};

const std::string engineBlunderTaunts[ENGINE_BLUNDER_TAUNTS_NO] = {
"Did I really play that?",
"I must have been distracted",
"Normally I don't do that",
"Just checking if you would notice that",
"What will you give me in return?",
"This is not what I usually do",
"Will be more careful next time",
"Didn't see that coming",
"How on Earth did you find THAT?",
"Show some gratitude",
"It wasn't me, it was my younger sibling",
"You got lucky",
"Feeling proud, eh?",
"Rodent's aren't supposed to be good at chess",
"I don't feel myself today",
"Stop hypnothysing me!",
"You might win this one, but tomorrow belongs to me!",
"Chess is a fairy tale of 1001 blunders",
};

// 

void PrintTaunt(int eventType) {

    Glob.currentTaunt = eventType;

    if (Glob.previousValue == 8888) {
        PrintGenericTaunt();
        return;
    }

    if (Glob.previousValue != 8888) {
        int delta = Glob.gameValue - Glob.previousValue;

        bool isSmallGain = delta > 30 && delta < 60;

        if (delta > 200) {
            PrintUserBlunderTaunt();
            return;
        }

        if (delta < -200) {
            PrintEngineBlunderTaunt();
            return;
        }

        if (isSmallGain && eventType == TAUNT_BALANCE) {
            PrintEngineEscapeTaunt();
            return;
        }

        if (isSmallGain && eventType == TAUNT_ADVANTAGE) {
            PrintGainingTaunt();
            return;
        }
    }

    if (eventType == TAUNT_CAPTURE)
        PrintCaptureTaunt();
    else if (eventType == TAUNT_WINNING)
        PrintWinningTaunt();
    else if (eventType == TAUNT_ADVANTAGE)
        PrintAdvantageTaunt();
    else if (eventType == TAUNT_BALANCE)
        PrintBalanceTaunt();
    else if (eventType == TAUNT_DISADVANTAGE)
        PrintDisdvantageTaunt();
    else if (eventType == TAUNT_LOSING)
        PrintLosingTaunt();
    else if (eventType == TAUNT_CRUSHING)
        PrintCrushingTaunt();
    else
        PrintGenericTaunt();
}


void PrintGenericTaunt() {
    srand(time(NULL));
    std::string word = generalTaunts[rand() % GENERAL_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintCaptureTaunt() {
    srand(time(NULL));
    std::string word = captureTaunts[rand() % CAPTURE_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintWinningTaunt() {
    srand(time(NULL));
    std::string word = winningTaunts[rand() % WINNING_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintAdvantageTaunt() {
    srand(time(NULL));
    std::string word = advantageTaunts[rand() % ADVANTAGE_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintBalanceTaunt() {
    srand(time(NULL));
    std::string word = balanceTaunts[rand() % BALANCE_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintDisdvantageTaunt() {
    srand(time(NULL));
    std::string word = disadvantageTaunts[rand() % DISADVANTAGE_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintLosingTaunt() {
    srand(time(NULL));
    std::string word = losingTaunts[rand() % LOSING_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintCrushingTaunt() {
    srand(time(NULL));
    std::string word = crushingTaunts[rand() % CRUSHING_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintUserBlunderTaunt() {
    srand(time(NULL));
    std::string word = userBlunderTaunts[rand() % USER_BLUNDER_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintEngineBlunderTaunt() {
    srand(time(NULL));
    std::string word = engineBlunderTaunts[rand() % ENGINE_BLUNDER_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintEngineEscapeTaunt() {
    srand(time(NULL));
    std::string word = escapeTaunts[rand() % ESCAPE_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}

void PrintGainingTaunt() {
    srand(time(NULL));
    std::string word = gainingTaunts[rand() % GAINING_TAUNTS_NO];
    printfUciOut("info string %s\n", word.c_str());
}