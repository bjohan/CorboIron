#!/bin/bash
cd download
wget https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/NewliquidCrystal_1.3.4.zip
unzip NewliquidCrystal_1.3.4.zip
ln -s download/NewliquidCrystal/LiquidCrystal_I2C.* .
ln -s download/NewliquidCrystal/I2CIO.* .
ln -s download/NewliquidCrystal/LCD.* .
