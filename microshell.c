#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void	err(char *str)
{
	while (*str)
	{
		write(2, str, 1);
		str++;
	}
}

int	cd(char **argv, int argc)
{
	if (argc != 2)
	{
		err("error: cd: bad arguments\n");
		return (1);
	}
	if (chdir(argv[1]) == -1)
	{
		err("error: cd: cannot change directory to ");
		err(argv[1]);
		err("\n");
		return (1);
	}
	return (0);
}

void	set_pipe(int has_pipe, int *fd, int end)
{
	if (has_pipe)
	{
		if (dup2(fd[end], end) == -1)
		{
			err("error: fatal\n");
			exit(1);
		}
		if (close(fd[0]) == -1)
		{
			err("error: fatal\n");
			exit(1);
		}
		if (close(fd[1]) == -1)
		{
			err("error: fatal\n");
			exit(1);
		}
	}
}

int	exec(char **argv, int i, char **envp)
{
	int	has_pipe = 0;
	int	fd[2];
	int	pid;
	int	status = 0;
	
	if (argv[i] && strcmp(argv[i], "|") == 0)
		has_pipe = 1;
	if (!has_pipe && strcmp(argv[0], "cd") == 0)
		return (cd(argv, i));
	if (has_pipe)
	{
		if (pipe(fd) == -1)
		{
			err("error: fatal\n");
			exit(1);
		}
	}
	pid = fork();
	if (pid == 0)
	{
		argv[i] = NULL;
		set_pipe(has_pipe, fd, 1);
		if (strcmp(argv[0], "cd") == 0)
			return (cd(argv, i));
		execve(argv[0], argv, envp);
		err("error: cannot execute ");
		err(argv[0]);
		err("\n");
		exit(1);
	}
	waitpid(pid, &status, 0);
	set_pipe(has_pipe, fd, 0);
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	else
		return (1);
}

int	main(int argc, char **argv, char **envp)
{
	int	status = 0;
	int	i = 1;
	
	while (argv[i])
	{
		argv += i;
		i = 0;
		while (argv[i] && strcmp(argv[i], "|") != 0 && strcmp(argv[i], ";") != 0)
			i++;
		if (i)
			status = exec(argv, i, envp);
		if (argv[i] != NULL)
			i++;
	}
	return (status);
}
