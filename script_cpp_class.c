#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

// doesn't print errors
static int create_file(char *name, char *suffix, char **file)
{
	*file = malloc(sizeof(char) * (strlen(name) + strlen(suffix) + 1));
	if (!*file)
		return ENOMEM;
	strcpy(*file, name);
	strcat(*file, suffix);
	int fd = open(*file, O_WRONLY | O_CREAT | O_EXCL, 00644);
	return fd;
}

// on error: print and close all
// exits on ENOMEM
static int create_both_files(char *name, int *fd_hpp, int *fd_cpp)
{
	char *file_hpp;
	char *file_cpp;

	*fd_hpp = create_file(name, ".hpp", &file_hpp);
	if (*fd_hpp == -1)
	{
		perror(file_hpp);
		free(file_hpp);
		return 1;
	}
	if (*fd_hpp == ENOMEM)
	{
		dprintf(2, "%s.hpp: %s\n", name, strerror(errno));
		exit(ENOMEM);
	}
	*fd_cpp = create_file(name, ".cpp", &file_cpp);
	if (*fd_cpp == -1)
	{
		perror(file_cpp);
		free(file_cpp);
		close(*fd_hpp);
		unlink(file_hpp);
		free(file_hpp);
		return 1;
	}
	if (*fd_cpp == ENOMEM)
	{
		dprintf(2, "%s.cpp: %s\n", name, strerror(errno));
		close(*fd_hpp);
		unlink(file_hpp);
		free(file_hpp);
		exit(ENOMEM);
	}
	printf("created %s and %s\n", file_hpp, file_cpp);
	free(file_hpp);
	free(file_cpp);
	return 0;
}

static void write_upr(int fd, char *str)
{
	char c;
	while (*str)
	{
		c = *str;
		if ('a' <= c && c <= 'z')
			c -= 'a' - 'A';
		write(fd, &c, 1);
		++str;
	}
}

static void write_hpp(int fd, char *name)
{
	dprintf(fd, "#ifndef "); write_upr(fd, name); dprintf(fd, "_H\n");
	dprintf(fd, "# define "); write_upr(fd, name); dprintf(fd, "_H\n");
	dprintf(fd, "\n");
	dprintf(fd, "# include <iostream>\n");
	dprintf(fd, "\n");
	dprintf(fd, "class %s\n", name);
	dprintf(fd, "{\n");
	dprintf(fd, "public:\n");
	dprintf(fd, "	%s();\n", name);
	dprintf(fd, "	%s(%s const& src);\n", name, name);
	dprintf(fd, "	~%s();\n", name);
	dprintf(fd, "	%s& operator=(%s const& rhs);\n", name, name);
	dprintf(fd, "};\n");
	dprintf(fd, "\n");
	dprintf(fd, "#endif\n");
}

static void write_cpp(int fd, char *name)
{
	dprintf(fd, "#include \"%s.hpp\"\n", name);
	dprintf(fd, "\n");
	dprintf(fd, "/* CONSTRUCTORS */\n");
	dprintf(fd, "%s::%s()\n", name, name);
	dprintf(fd, "{}\n");
	dprintf(fd, "\n");
	dprintf(fd, "%s::%s(%s const& src)\n", name, name, name);
	dprintf(fd, "{\n");
	dprintf(fd, "	this->operator=(src);\n");
	dprintf(fd, "}\n");
	dprintf(fd, "\n");
	dprintf(fd, "%s::~%s()\n", name, name);
	dprintf(fd, "{}\n");
	dprintf(fd, "\n");
	dprintf(fd, "%s& %s::operator=(%s const& rhs)\n", name, name, name);
	dprintf(fd, "{\n");
	dprintf(fd, "	return *this;\n");
	dprintf(fd, "}\n");
	dprintf(fd, "\n");
	dprintf(fd, "/* G(/S)ETTERS */\n");
	dprintf(fd, "\n");
	dprintf(fd, "/* MEMBERS */\n");
}

static int input_checks(char *name)
{
	if (!*name)
		return 1;
	return 0;
}

int main(int ac, char **av)
{
	if (ac == 1)
		return printf("Error: provide at least one class name\n"), 1;
	
	int fd_hpp;
	int fd_cpp;
	for (int i = 1; i < ac; ++i)
	{
		if (input_checks(av[i]))
			continue ;
		if (create_both_files(av[i], &fd_hpp, &fd_cpp))
			continue ;
		write_hpp(fd_hpp, av[i]);
		write_cpp(fd_cpp, av[i]);
		close(fd_hpp);
		close(fd_cpp);
	}
}
