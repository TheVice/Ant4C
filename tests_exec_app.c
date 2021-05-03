/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 TheVice
 *
 */

#include <stdio.h>
#include <stdlib.h>

#if 1/*TODO:*/
int main(int argc, char** argv, char** envp)
{
#else
int main(int argc, char** argv)

{
	char** envp = environ;
#endif
	printf("\n<App4ExecTest>\n\t<arguments>\n");

	for (int i = 0; i < argc; ++i)
	{
		printf("\t\t<argument><![CDATA[%s]]></argument>\n", argv[i]);
	}

	/*TODO: Working directory.*/
	argc = 0;
	printf("\t</arguments>\n\t<environments>\n");

	while (NULL != envp[argc])
	{
		printf("\t\t<environment><![CDATA[%s]]></environment>\n", envp[argc++]);
	}

	printf("\t</environments>\n</App4ExecTest>\n");
	return EXIT_SUCCESS;
}
