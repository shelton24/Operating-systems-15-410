# To build your project, edit this file--not the Makefile!

##################################################
# How to acquire updates.
##################################################
#
# afs - fastest on Andrew machines
# web - will probably work on non-Andrew machines
# offline - if machine has no network access
#
# WARNING - if you miss a staff-issued update
# because you were using "offline", you are
# responsible for the outcome.
#
##################################################
UPDATE_METHOD = afs

##################################################
# Tab stops
##################################################
# If you use tabstops set to something other than
# the international standard of eight characters,
# this is your opportunity to inform our print
# scripts.
TABSTOP = 8

##################################################
# Object files from kern/ which the game AND the
# console tester should link against -- basically,
# the object files which make up your drivers.
##################################################
#
COMMON_OBJS = fake.o console_device_driver.o install_handlers.o interrupt_handler_wrappers.o

##################################################
# Object files from 410kern/ for just the game
# (in other words, any game-specific helper code
# we release to you--some semesters, there isn't
# any).
##################################################
#
410_GAME_OBJS =

##################################################
# Object files from kern/ for just the game:
# this allows you to break up your game into
# multiple parts.
##################################################
#
KERN_GAME_OBJS = game.o

##################################################
# Object files from 410kern/ for just the tester
# (you should not need to change this).
##################################################
#
410TEST_OBJS = 410_test.o
