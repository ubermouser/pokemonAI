#!/usr/bin/env python3
import json
import sys

STATVAL = ["atk", "spa", "def", "spd", "spe", "hp"]

def convert_pokemon(line):
    line = line.split("\t")
    pokemon = {
        "header": "PKAIP0",
        "name": line[1],
        "species": line[2],
        "level": int(line[3]),
        "item": line[4],
        "sex": int(line[5]),
        "ability": line[6],
        "nature": line[7],
    }
    pokemon["moves"] = [line[8+idx] for idx in range(4) if line[8+idx] != "NONE"]
    pokemon["iv"] = {STATVAL[idx]: int(line[12+idx]) for idx in range(6)}
    pokemon["ev"] = {STATVAL[idx]: int(line[18+idx]) for idx in range(6)}
    return pokemon
    
def convert_team(lines):
    header = lines[0].split("\t")
    team = {
        "header": "PKAIE0",
        "name": header[1],
        "pokemon": [convert_pokemon(line) for line in lines[1:]]
    }
    return team
    
    
def main(inpath, outpath):
    with open(inpath) as o:
        infile = o.read()
    
    inlines = infile.split("\n")
    team = convert_team(inlines)
    json.dump(team, open(outpath, mode='w'), indent=2)
    
    
if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
