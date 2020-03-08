# rodent-iv
trying to start a community effort based on Rodent IV

# rodent-iv
trying to start a community effort based on Rodent IV

If You add something to Rodent IV, keep in mind the following:

1. It's easier to work with small patches. They are more readable and cause
   less problems in case of an error.
   
2. Starting with bugfixes reduces headache. Fixing things before adding some
   great idea might accidentally do something good to implementation of Your
   idea as well. Going the other way round causes bugs to breed.

3. If fixing a bug required writing some diagnostic code, then diagnostic 
   code stays (perhaps under an #ifdef) because it might help with 
   yet another bug.

4. With a non-functional patch, please test that it is non-functional: run 
   bench command and compare node counts before and after the patch.

5. With an Elo-gaining patch, please run a test of at least 1000 games,
   preferably more.

6. With a feature-adding patch it gets more difficult. Strength is not 
   the main goal of Rodent project, but creating personalities requires
   some Elo to be thrown to the burner, sometimes in hundreds. And I would
   like most personalities to be able to beat Fruit 2.1 to ensure they are
   of decent GM strength. Default Rodent should at least keep 2900-3000 CCRL
   rating. Improvement is nice, but optional.
   
7. Treat Rodent like legacy software. A lot of things in there require
   refactoring, code quality is worse than chess quality, so tread carefully.
   I had no training in IT when I started this project. It taught me enough
   to get a job - and to notice that some of my code sucks.
   
8. Don't overdo automatic tuning. Rodent III lost a lot of style because of that,
   and I had to revert many changes.
   
9. Please don't remove quirks and features. They make Rodent what it is.

10. Enjoy. Rodent is meant to be fun.

