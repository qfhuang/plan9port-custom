#ifndef _9PCLIENT_H_
#define _9PCLIENT_H_ 1
#ifdef __cplusplus
extern "C" {
#endif

AUTOLIB(9pclient)
/*
 * Simple user-level 9P client.
 */

typedef struct CFsys CFsys;
typedef struct CFid CFid;

CFsys *fsinit(int);
CFsys *fsmount(int, char*);

int fsversion(CFsys*, int, char*, int);
CFid *fsauth(CFsys*, char*);
CFid *fsattach(CFsys*, CFid*, char*, char*);
CFid *fsopen(CFsys*, char*, int);
int fsopenfd(CFsys*, char*, int);
long fsread(CFid*, void*, long);
long fsreadn(CFid*, void*, long);
long fswrite(CFid*, void*, long);
void fsclose(CFid*);
void fsunmount(CFsys*);
struct Dir;	/* in case there's no lib9.h */
long fsdirread(CFid*, struct Dir**);
long fsdirreadall(CFid*, struct Dir**);
struct Dir *fsdirstat(CFsys*, char*);
struct Dir *fsdirfstat(CFid*);
int fsdirwstat(CFsys*, char*, struct Dir*);
int fsdirfwstat(CFid*, struct Dir*);
CFid *fsroot(CFsys*);
void fssetroot(CFsys*, CFid*);
CFsys *nsmount(char*, char*);

#ifdef __cplusplus
}
#endif
#endif