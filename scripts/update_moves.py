import csv

input_file = '/workspace/data/gen1/moves.csv'
output_file = '/workspace/data/gen1/moves.csv'

# Target categories
SELF = {
    'Swords Dance', 'Growth', 'Meditate', 'Agility', 'Teleport', 'Double Team', 'Recover', 
    'Harden', 'Minimize', 'Withdraw', 'Defense Curl', 'Barrier', 'Focus Energy', 'Bide', 
    'Amnesia', 'SoftBoiled', 'Splash', 'Acid Armor', 'Rest', 'Sharpen', 
    'Conversion', 'Substitute', 'Protect', 'Belly Drum', 'Destiny Bond', 'Detect', 
    'Endure', 'Milk Drink', 'Morning Sun', 'Synthesis', 
    'Moonlight', 'Stockpile', 'Swallow', 'Follow Me', 'Charge', 'Wish', 'Assist', 
    'Ingrain', 'Magic Coat', 'Recycle', 'Imprison', 'Refresh', 'Grudge', 'Snatch', 
    'Camouflage', 'Tail Glow', 'Slack Off', 'Cosmic Power', 'Iron Defense', 
    'Howl', 'Bulk Up', 'Calm Mind', 'Dragon Dance', 'Roost', 'Healing Wish', 
    'Power Trick', 'Copycat', 'Aqua Ring', 'Magnet Rise', 'Rock Polish', 'Nasty Plot', 
    'Defend Order', 'Heal Order', 'Lunar Dance', 'Hone Claws', 'Autotomize', 'Rage Powder', 
    'Quiver Dance', 'Coil', 'Ally Switch', 'Shell Smash', 'Shift Gear', 'Work Up', 
    'Cotton Guard', 'King\'s Shield', 'Spiky Shield', 'Geomancy', 'Celebrate', 
    'Shore Up', 'Baneful Bunker', 'Laser Focus', 'Extreme Evoboost', 'Max Guard', 
    'Stuff Cheeks', 'No Retreat', 'Clangorous Soul', 'Obstruct', 'Power Shift', 
    'Victory Dance', 'Shelter', 'Take Heart', 'Silk Trap', 'Revival Blessing', 
    'Fillet Away', 'Shed Tail', 'Tidy Up', 'Burning Bulwark',
}
ANY_ADJACENT_ALLY = {
    'Helping Hand', 'Aromatic Mist', 'Hold Hands',
}
ANY_ADJACENT_ALLY_SELF = {
    'Acupressure'
}
ANY_ADJACENT_ENEMY = {
    'Me First', 'Max Flare', 'Max Flutterby', 'Max Lightning', 'Max Strike', 'Max Knuckle', 
    'Max Phantasm', 'Max Hailstorm', 'Max Ooze', 'Max Geyser', 'Max Airstream', 
    'Max Starfall', 'Max Wyrmwind', 'Max Mindstorm', 'Max Rockfall', 'Max Quake', 
    'Max Darkness', 'Max Overgrowth', 'Max Steelspike', 'Doodle',
    # Outrage moves:
    'Outrage', 'Thrash', 'Petal Dance',
    # Default moves:
    'Struggle',
}
ANY_ACTIVE = {
    'Gust', 'Wing Attack', 'Fly', 'Peck', 'Drill Peck', 'Sky Attack', 'Aeroblast', 
    'Aerial Ace', 'Bounce', 'Water Pulse', 'Pluck', 'Aura Sphere', 'Dark Pulse', 
    'Air Slash', 'Dragon Pulse', 'Brave Bird', 'Chatter', 'Heal Pulse', 'Sky Drop', 
    'Acrobatics', 'Hurricane', 'Flying Press', 'Oblivion Wing', 'Dragon Ascent',
}
ANY_ALLY = {
    'Baton Pass'
}
ANY_ALLY_SELF = {
    'U-turn',
}
ALL_ADJACENT = {
    'Surf', 'Earthquake', 'SelfDestruct', 'Explosion', 'Magnitude', 'Teeter Dance', 
    'Discharge', 'Lava Plume', 'Sludge Wave', 'Synchronoise', 'Bulldoze', 'Searing Shot', 
    'Parabolic Charge', 'Petal Blizzard', 'Boomburst', 'Sparkling Aria', 'Brutal Swing', 
    'Mind Blown', 'Misty Explosion', 'Corrosive Gas',
}
ALL_ADJACENT_ENEMY = { 
    'Razor Wind', 'Tail Whip', 'Leer', 'Growl', 'Acid', 'Blizzard', 'Razor Leaf', 
    'String Shot', 'Swift', 'Poison Gas', 'Bubble', 'Rock Slide', 'Cotton Spore', 
    'Powder Snow', 'Icy Wind', 'Sweet Scent', 'Twister', 'Heat Wave', 'Eruption', 
    'Hyper Voice', 'Air Cutter', 'Water Spout', 'Muddy Water', 'Heal Block', 'Captivate', 
    'Dark Void', 'Incinerate', 'Struggle Bug', 'Electroweb', 'Relic Song', 'Glaciate', 
    'Snarl', 'Disarming Voice', 'Diamond Storm', 'Venom Drench', 'Dazzling Gleam', 
    'Thousand Arrows', 'Thousand Waves', 'Land\'s Wrath', 'Origin Pulse', 
    'Precipice Blades', 'Core Enforcer', 'Clanging Scales', 'Shell Trap', 
    'Clangorous Soulblaze', 'Splishy Splash', 'Breaking Swipe', 'Overdrive', 
    'Burning Jealousy', 'Dragon Energy', 'Fiery Wrath', 'Glacial Lance', 'Astral Barrage', 
    'Springtide Storm', 'Bleakwind Storm', 'Wildbolt Storm', 'Sandsear Storm', 'Mortal Spin', 
    'Make It Rain', 'Matcha Gotcha', 'Nihil Light',
}
ALL_ALLIES = {
    'Heal Bell', 'Aromatherapy', 'Magnetic Flux',
}
ALL_ACTIVE = {
    'Perish Song',
}
SIDE_ALL = {
    'Haze', 'Sandstorm', 'Rain Dance', 'Sunny Day', 'Hail', 'Mud Sport', 'Water Sport', 
    'Gravity', 'Trick Room', 'Wonder Room', 'Magic Room', 'Ion Deluge', 'Grassy Terrain', 
    'Misty Terrain', 'Fairy Lock', 'Electric Terrain', 'Psychic Terrain', 'Court Change', 
    'Chilly Reception', 'Snowscape', 
}
SIDE_ALLY = {
    'Mist', 'Light Screen', 'Reflect', 'Safeguard', 'Tailwind', 'Lucky Chant', 
    'Wide Guard', 'Quick Guard', 'Mat Block', 'Crafty Shield', 'Happy Hour', 
    'Aurora Veil',
}
SIDE_ENEMY = {
    'Spikes', 'Toxic Spikes', 'Stealth Rock', 'Sticky Web',
}
UNKNOWN = {
    'Counter', 'Curse', 'Mirror Coat', 'Nature Power', 'Metal Burst', 'Comeuppance', 
    'Metronome', 'Sleep Talk'
}

TARGET_TYPE_MAP = {
    'SELF': SELF,
    # 'ANY_ADJACENT': ANY_ADJACENT,
    'ANY_ADJACENT_ALLY': ANY_ADJACENT_ALLY,
    'ANY_ADJACENT_ENEMY': ANY_ADJACENT_ENEMY,
    'ANY_ADJACENT_ALLY_SELF': ANY_ADJACENT_ALLY_SELF,
    'ANY_ACTIVE': ANY_ACTIVE,
    'ANY_ALLY': ANY_ALLY,
    'ANY_ALLY_SELF': ANY_ALLY_SELF,
    'ALL_ADJACENT': ALL_ADJACENT,
    'ALL_ADJACENT_ENEMY': ALL_ADJACENT_ENEMY,
    'ALL_ALLIES': ALL_ALLIES,
    'ALL_ACTIVE': ALL_ACTIVE,
    'SIDE_ALL': SIDE_ALL,
    'SIDE_ALLY': SIDE_ALLY,
    'SIDE_ENEMY': SIDE_ENEMY,
    'UNKNOWN': UNKNOWN,
}

with open(input_file, 'r', newline='') as f:
    reader = csv.reader(f, delimiter='\t')
    header = next(reader)
    rows = []
    for row in reader:
        name = row[0]
        prev_target = row[6]
        
        row_found = False
        for target_type, moves in TARGET_TYPE_MAP.items():
            moves = {m.lower().strip() for m in moves}
            if name.lower().strip() in moves:
                row[6] = target_type
                row_found = True
                break
        if not row_found:
            row[6] = 'ANY_ADJACENT'

        if prev_target != row[6]:
            print(f"{name}: {prev_target} -> {row[6]}")
        rows.append(row)

with open(output_file, 'w', newline='') as f:
    writer = csv.writer(f, delimiter='\t')
    writer.writerow(header)
    writer.writerows(rows)
