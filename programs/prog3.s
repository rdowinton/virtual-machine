.entryp main;
.section data;
	string txt "Random numbers: ";
	string space " ";
	string nl "\n";
.section code;
	.sub main;
		rand %g1;
		rand %g2;
		print @txt;
		print %g1;
		print @space;
		print %g2;
		print @nl;
	.ends;
