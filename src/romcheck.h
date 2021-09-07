/* list stolen from mgba src/gba/overrides.c */
static int is_mbit_rom(char *id) {
	static const char mbit_roms[][4] = {
		"PEAJ", "PSAJ", "PSAE", "BFTJ", "AXVJ",
		"AXVE", "AXVP", "AXVI", "AXVS", "AXVD",
		"AXVF", "AXPJ", "AXPE", "AXPP", "AXPI",
		"AXPS", "AXPD", "AXPF", "BPEJ", "BPEE",
		"BPEP", "BPEI", "BPES", "BPED", "BPEF",
		"B24J", "B24E", "B24P", "B24U", "BPRJ",
		"BPRE", "BPRP", "BPRI", "BPRS", "BPRD",
		"BPRF", "BPGJ", "BPGE", "BPGP", "BPGI",
		"BPGS", "BPGD", "BPGF", "BKAJ", "AX4J",
		"AX4E", "AX4P",


		"\0\0\0\0"
	};
	u32 i;
	for(i=0; mbit_roms[i][0]; ++i)
		if(!strncmp(id, mbit_roms[i], 4)) return 1;
	return 0;
}
