#include <pipe-fork.h>
#include <unistd.h> // pipe
#include <sys/types.h> // pid_t


// fork new process assigning descriptors of four communicating pipes
// for parent and child.
// returns pid from fork(2) or -1 on error.
int pipe4_fork(int *pipe1_wr, int *pipe1_rd, int *pipe2_wr, int *pipe2_rd)
{
    *pipe1_wr = -1;
    *pipe1_rd = -1;
    *pipe2_wr = -1;
    *pipe2_rd = -1;

    int      parent_child1[2]; // 1 parent -> child
    if (pipe(parent_child1) < 0)
	return -1;

    int      child_parent1[2]; // 1 parent <- child
    if (pipe(child_parent1) < 0)
	return -1;

    int      parent_child2[2]; // 2 parent -> child
    if (pipe(parent_child2) < 0)
	return -1;

    int      child_parent2[2]; // 2 parent <- child
    if (pipe(child_parent2) < 0)
	return -1;

    pid_t pid = fork();
    if (pid < 0)
	return pid;

    if (pid != 0)   // parent
    {
	close(      parent_child1[0]);  // 1 read  end
	*pipe1_wr = parent_child1[1];   // 1 write end

	close(      child_parent1[1]);  // 1 write end
	*pipe1_rd = child_parent1[0];   // 1 read  end


	close(      parent_child2[0]);  // 2 read  end
	*pipe2_wr = parent_child2[1];   // 2 write end

	close(      child_parent2[1]);  // 2 write end
	*pipe2_rd = child_parent2[0];   // 2 read  end
    }
    else            // child
    {
	close(      child_parent1[0]);  // 1 read  end
	*pipe1_wr = child_parent1[1];   // 1 write end

	close(      parent_child1[1]);  // 1 write end
	*pipe1_rd = parent_child1[0];   // 1 read  end


	close(      child_parent2[0]);  // 2 read  end
	*pipe2_wr = child_parent2[1];   // 2 write end

	close(      parent_child2[1]);  // 2 write end
	*pipe2_rd = parent_child2[0];   // 2 read  end
    }

    return pid;
}
