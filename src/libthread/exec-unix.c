#include <fcntl.h>
#include <unistd.h>
#include "threadimpl.h"

void
procexec(Channel *pidc, char *prog, char *args[])
{
	int n;
	Proc *p;
	Thread *t;

print("procexec\n");
	_threaddebug(DBGEXEC, "procexec %s", prog);
	/* must be only thread in proc */
	p = _threadgetproc();
	t = p->thread;
	if(p->threads.head != t || p->threads.head->nextt != nil){
print("not only thread\n");
		werrstr("not only thread in proc");
	Bad:
		if(pidc)
			sendul(pidc, ~0);
		return;
	}

	/*
	 * We want procexec to behave like exec; if exec succeeds,
	 * never return, and if it fails, return with errstr set.
	 * Unfortunately, the exec happens in another proc since
	 * we have to wait for the exec'ed process to finish.
	 * To provide the semantics, we open a pipe with the 
	 * write end close-on-exec and hand it to the proc that
	 * is doing the exec.  If the exec succeeds, the pipe will
	 * close so that our read below fails.  If the exec fails,
	 * then the proc doing the exec sends the errstr down the
	 * pipe to us.
	 */
	if(pipe(p->exec.fd) < 0)
{
print("pipe\n");
		goto Bad;
}
	if(fcntl(p->exec.fd[1], F_SETFD, 1) < 0)
{
print("fcntl\n");
		goto Bad;
}

	/* exec in parallel via the scheduler */
	assert(p->needexec==0);
	p->exec.prog = prog;
	p->exec.args = args;
	p->needexec = 1;
print("sched\n");
	_sched();

	close(p->exec.fd[1]);
	if((n = read(p->exec.fd[0], p->exitstr, ERRMAX-1)) > 0){	/* exec failed */
		p->exitstr[n] = '\0';
print("read got %s\n", p->exitstr);
		errstr(p->exitstr, ERRMAX);
		close(p->exec.fd[0]);
		goto Bad;
	}
	close(p->exec.fd[0]);

print("exec %d\n", pidc);
	if(pidc)
		sendul(pidc, t->ret);

	/* wait for exec'ed program, then exit */
	_schedexecwait();
}

void
procexecl(Channel *pidc, char *f, ...)
{
	procexec(pidc, f, &f+1);
}

void
_schedexecwait(void)
{
	int pid;
	Channel *c;
	Proc *p;
	Thread *t;
	Waitmsg *w;

	p = _threadgetproc();
	t = p->thread;
	pid = t->ret;
	_threaddebug(DBGEXEC, "_schedexecwait %d", t->ret);

	for(;;){
		w = wait();
		if(w == nil)
			break;
		if(w->pid == pid)
			break;
		free(w);
	}
	if(w != nil){
		if((c = _threadwaitchan) != nil)
			sendp(c, w);
		else
			free(w);
	}
	threadexits("procexec");
}

static void
efork(void *ve)
{
	char buf[ERRMAX];
	Execargs *e;

	e = ve;
	_threaddebug(DBGEXEC, "_schedexec %s", e->prog);
	close(e->fd[0]);
	execv(e->prog, e->args);
	_threaddebug(DBGEXEC, "_schedexec failed: %r");
	rerrstr(buf, sizeof buf);
	if(buf[0]=='\0')
		strcpy(buf, "exec failed");
	write(e->fd[1], buf, strlen(buf));
	close(e->fd[1]);
	_exits(buf);
}

int
_schedexec(Execargs *e)
{
	return ffork(RFFDG|RFPROC|RFMEM, efork, e);
}
