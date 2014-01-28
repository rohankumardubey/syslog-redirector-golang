// auto generated by go tool dist
// goos=linux goarch=amd64

#include "runtime.h"
#include "arch_GOARCH.h"
#include "malloc.h"
#include "defs_GOOS_GOARCH.h"
#include "type.h"

#line 16 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
static Lock proflock; 
#line 21 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
enum { MProf , BProf } ; 
#line 25 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
typedef struct Bucket Bucket; 
struct Bucket 
{ 
Bucket *next; 
Bucket *allnext; 
int32 typ; 
#line 33 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
union 
{ 
struct 
{ 
uintptr allocs; 
uintptr frees; 
uintptr alloc_bytes; 
uintptr free_bytes; 
uintptr recent_allocs; 
uintptr recent_frees; 
uintptr recent_alloc_bytes; 
uintptr recent_free_bytes; 
} ; 
struct 
{ 
int64 count; 
int64 cycles; 
} ; 
} ; 
uintptr hash; 
uintptr nstk; 
uintptr stk[1]; 
} ; 
enum { 
BuckHashSize = 179999 , 
} ; 
static Bucket **buckhash; 
static Bucket *mbuckets; 
static Bucket *bbuckets; 
static uintptr bucketmem; 
#line 65 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
static Bucket* 
stkbucket ( int32 typ , uintptr *stk , int32 nstk , bool alloc ) 
{ 
int32 i; 
uintptr h; 
Bucket *b; 
#line 72 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
if ( buckhash == nil ) { 
buckhash = runtime·SysAlloc ( BuckHashSize*sizeof buckhash[0] , &mstats.buckhash_sys ) ; 
if ( buckhash == nil ) 
runtime·throw ( "runtime: cannot allocate memory" ) ; 
} 
#line 79 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
h = 0; 
for ( i=0; i<nstk; i++ ) { 
h += stk[i]; 
h += h<<10; 
h ^= h>>6; 
} 
h += h<<3; 
h ^= h>>11; 
#line 88 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
i = h%BuckHashSize; 
for ( b = buckhash[i]; b; b=b->next ) 
if ( b->typ == typ && b->hash == h && b->nstk == nstk && 
runtime·mcmp ( ( byte* ) b->stk , ( byte* ) stk , nstk*sizeof stk[0] ) == 0 ) 
return b; 
#line 94 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
if ( !alloc ) 
return nil; 
#line 97 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
b = runtime·persistentalloc ( sizeof *b + nstk*sizeof stk[0] , 0 , &mstats.buckhash_sys ) ; 
bucketmem += sizeof *b + nstk*sizeof stk[0]; 
runtime·memmove ( b->stk , stk , nstk*sizeof stk[0] ) ; 
b->typ = typ; 
b->hash = h; 
b->nstk = nstk; 
b->next = buckhash[i]; 
buckhash[i] = b; 
if ( typ == MProf ) { 
b->allnext = mbuckets; 
mbuckets = b; 
} else { 
b->allnext = bbuckets; 
bbuckets = b; 
} 
return b; 
} 
#line 115 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
static void 
MProf_GC ( void ) 
{ 
Bucket *b; 
#line 120 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
for ( b=mbuckets; b; b=b->allnext ) { 
b->allocs += b->recent_allocs; 
b->frees += b->recent_frees; 
b->alloc_bytes += b->recent_alloc_bytes; 
b->free_bytes += b->recent_free_bytes; 
b->recent_allocs = 0; 
b->recent_frees = 0; 
b->recent_alloc_bytes = 0; 
b->recent_free_bytes = 0; 
} 
} 
#line 133 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
void 
runtime·MProf_GC ( void ) 
{ 
runtime·lock ( &proflock ) ; 
MProf_GC ( ) ; 
runtime·unlock ( &proflock ) ; 
} 
#line 149 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
typedef struct AddrHash AddrHash; 
typedef struct AddrEntry AddrEntry; 
#line 152 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
enum { 
AddrHashBits = 12 , 
AddrHashShift = 20 , 
AddrDenseBits = 8 , 
} ; 
#line 158 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
struct AddrHash 
{ 
AddrHash *next; 
uintptr addr; 
AddrEntry *dense[1<<AddrDenseBits]; 
} ; 
#line 165 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
struct AddrEntry 
{ 
AddrEntry *next; 
uint32 addr; 
Bucket *b; 
} ; 
#line 172 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
static AddrHash **addrhash; 
static AddrEntry *addrfree; 
static uintptr addrmem; 
#line 181 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
enum { 
HashMultiplier = 2654435769U 
} ; 
#line 186 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
static void 
setaddrbucket ( uintptr addr , Bucket *b ) 
{ 
int32 i; 
uint32 h; 
AddrHash *ah; 
AddrEntry *e; 
#line 194 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
h = ( uint32 ) ( ( addr>>AddrHashShift ) *HashMultiplier ) >> ( 32-AddrHashBits ) ; 
for ( ah=addrhash[h]; ah; ah=ah->next ) 
if ( ah->addr == ( addr>>AddrHashShift ) ) 
goto found; 
#line 199 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
ah = runtime·persistentalloc ( sizeof *ah , 0 , &mstats.buckhash_sys ) ; 
addrmem += sizeof *ah; 
ah->next = addrhash[h]; 
ah->addr = addr>>AddrHashShift; 
addrhash[h] = ah; 
#line 205 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
found: 
if ( ( e = addrfree ) == nil ) { 
e = runtime·persistentalloc ( 64*sizeof *e , 0 , &mstats.buckhash_sys ) ; 
addrmem += 64*sizeof *e; 
for ( i=0; i+1<64; i++ ) 
e[i].next = &e[i+1]; 
e[63].next = nil; 
} 
addrfree = e->next; 
e->addr = ( uint32 ) ~ ( addr & ( ( 1<<AddrHashShift ) -1 ) ) ; 
e->b = b; 
h = ( addr>> ( AddrHashShift-AddrDenseBits ) ) & ( nelem ( ah->dense ) -1 ) ; 
e->next = ah->dense[h]; 
ah->dense[h] = e; 
} 
#line 222 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
static Bucket* 
getaddrbucket ( uintptr addr ) 
{ 
uint32 h; 
AddrHash *ah; 
AddrEntry *e , **l; 
Bucket *b; 
#line 230 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
h = ( uint32 ) ( ( addr>>AddrHashShift ) *HashMultiplier ) >> ( 32-AddrHashBits ) ; 
for ( ah=addrhash[h]; ah; ah=ah->next ) 
if ( ah->addr == ( addr>>AddrHashShift ) ) 
goto found; 
return nil; 
#line 236 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
found: 
h = ( addr>> ( AddrHashShift-AddrDenseBits ) ) & ( nelem ( ah->dense ) -1 ) ; 
for ( l=&ah->dense[h]; ( e=*l ) != nil; l=&e->next ) { 
if ( e->addr == ( uint32 ) ~ ( addr & ( ( 1<<AddrHashShift ) -1 ) ) ) { 
*l = e->next; 
b = e->b; 
e->next = addrfree; 
addrfree = e; 
return b; 
} 
} 
return nil; 
} 
#line 251 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
void 
runtime·MProf_Malloc ( void *p , uintptr size ) 
{ 
int32 nstk; 
uintptr stk[32]; 
Bucket *b; 
#line 258 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
nstk = runtime·callers ( 1 , stk , 32 ) ; 
runtime·lock ( &proflock ) ; 
b = stkbucket ( MProf , stk , nstk , true ) ; 
b->recent_allocs++; 
b->recent_alloc_bytes += size; 
setaddrbucket ( ( uintptr ) p , b ) ; 
runtime·unlock ( &proflock ) ; 
} 
#line 268 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
void 
runtime·MProf_Free ( void *p , uintptr size ) 
{ 
Bucket *b; 
#line 273 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
runtime·lock ( &proflock ) ; 
b = getaddrbucket ( ( uintptr ) p ) ; 
if ( b != nil ) { 
b->recent_frees++; 
b->recent_free_bytes += size; 
} 
runtime·unlock ( &proflock ) ; 
} 
#line 282 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
int64 runtime·blockprofilerate; 
#line 284 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
void 
runtime·SetBlockProfileRate ( intgo rate ) 
{ 
int64 r; 
#line 289 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
if ( rate <= 0 ) 
r = 0; 
else { 
#line 293 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
r = ( float64 ) rate*runtime·tickspersecond ( ) / ( 1000*1000*1000 ) ; 
if ( r == 0 ) 
r = 1; 
} 
runtime·atomicstore64 ( ( uint64* ) &runtime·blockprofilerate , r ) ; 
} 
#line 300 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
void 
runtime·blockevent ( int64 cycles , int32 skip ) 
{ 
int32 nstk; 
int64 rate; 
uintptr stk[32]; 
Bucket *b; 
#line 308 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
if ( cycles <= 0 ) 
return; 
rate = runtime·atomicload64 ( ( uint64* ) &runtime·blockprofilerate ) ; 
if ( rate <= 0 || ( rate > cycles && runtime·fastrand1 ( ) %rate > cycles ) ) 
return; 
#line 314 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
nstk = runtime·callers ( skip , stk , 32 ) ; 
runtime·lock ( &proflock ) ; 
b = stkbucket ( BProf , stk , nstk , true ) ; 
b->count++; 
b->cycles += cycles; 
runtime·unlock ( &proflock ) ; 
} 
#line 325 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
typedef struct Record Record; 
struct Record { 
int64 alloc_bytes , free_bytes; 
int64 alloc_objects , free_objects; 
uintptr stk[32]; 
} ; 
#line 333 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
static void 
record ( Record *r , Bucket *b ) 
{ 
int32 i; 
#line 338 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
r->alloc_bytes = b->alloc_bytes; 
r->free_bytes = b->free_bytes; 
r->alloc_objects = b->allocs; 
r->free_objects = b->frees; 
for ( i=0; i<b->nstk && i<nelem ( r->stk ) ; i++ ) 
r->stk[i] = b->stk[i]; 
for ( ; i<nelem ( r->stk ) ; i++ ) 
r->stk[i] = 0; 
} 
void
runtime·MemProfile(Slice p, bool include_inuse_zero, uint8, uint16, uint32, intgo n, bool ok)
{
#line 348 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"

	Bucket *b;
	Record *r;
	bool clear;

	runtime·lock(&proflock);
	n = 0;
	clear = true;
	for(b=mbuckets; b; b=b->allnext) {
		if(include_inuse_zero || b->alloc_bytes != b->free_bytes)
			n++;
		if(b->allocs != 0 || b->frees != 0)
			clear = false;
	}
	if(clear) {
		// Absolutely no data, suggesting that a garbage collection
		// has not yet happened. In order to allow profiling when
		// garbage collection is disabled from the beginning of execution,
		// accumulate stats as if a GC just happened, and recount buckets.
		MProf_GC();
		n = 0;
		for(b=mbuckets; b; b=b->allnext)
			if(include_inuse_zero || b->alloc_bytes != b->free_bytes)
				n++;
	}
	ok = false;
	if(n <= p.len) {
		ok = true;
		r = (Record*)p.array;
		for(b=mbuckets; b; b=b->allnext)
			if(include_inuse_zero || b->alloc_bytes != b->free_bytes)
				record(r++, b);
	}
	runtime·unlock(&proflock);
	FLUSH(&n);
	FLUSH(&ok);
}

#line 385 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
typedef struct BRecord BRecord; 
struct BRecord { 
int64 count; 
int64 cycles; 
uintptr stk[32]; 
} ; 
void
runtime·BlockProfile(Slice p, intgo n, bool ok)
{
#line 392 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"

	Bucket *b;
	BRecord *r;
	int32 i;

	runtime·lock(&proflock);
	n = 0;
	for(b=bbuckets; b; b=b->allnext)
		n++;
	ok = false;
	if(n <= p.len) {
		ok = true;
		r = (BRecord*)p.array;
		for(b=bbuckets; b; b=b->allnext, r++) {
			r->count = b->count;
			r->cycles = b->cycles;
			for(i=0; i<b->nstk && i<nelem(r->stk); i++)
				r->stk[i] = b->stk[i];
			for(; i<nelem(r->stk); i++)
				r->stk[i] = 0;			
		}
	}
	runtime·unlock(&proflock);
	FLUSH(&n);
	FLUSH(&ok);
}

#line 418 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
typedef struct TRecord TRecord; 
struct TRecord { 
uintptr stk[32]; 
} ; 
void
runtime·ThreadCreateProfile(Slice p, intgo n, bool ok)
{
#line 423 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"

	TRecord *r;
	M *first, *mp;
	
	first = runtime·atomicloadp(&runtime·allm);
	n = 0;
	for(mp=first; mp; mp=mp->alllink)
		n++;
	ok = false;
	if(n <= p.len) {
		ok = true;
		r = (TRecord*)p.array;
		for(mp=first; mp; mp=mp->alllink) {
			runtime·memmove(r->stk, mp->createstack, sizeof r->stk);
			r++;
		}
	}
	FLUSH(&n);
	FLUSH(&ok);
}
void
runtime·Stack(Slice b, bool all, uint8, uint16, uint32, intgo n)
{
#line 442 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"

	uintptr pc, sp;
	
	sp = runtime·getcallersp(&b);
	pc = (uintptr)runtime·getcallerpc(&b);

	if(all) {
		runtime·semacquire(&runtime·worldsema, false);
		m->gcing = 1;
		runtime·stoptheworld();
	}

	if(b.len == 0)
		n = 0;
	else{
		g->writebuf = (byte*)b.array;
		g->writenbuf = b.len;
		runtime·goroutineheader(g);
		runtime·traceback(pc, sp, 0, g);
		if(all)
			runtime·tracebackothers(g);
		n = b.len - g->writenbuf;
		g->writebuf = nil;
		g->writenbuf = 0;
	}
	
	if(all) {
		m->gcing = 0;
		runtime·semrelease(&runtime·worldsema);
		runtime·starttheworld();
	}
	FLUSH(&n);
}

#line 475 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
static void 
saveg ( uintptr pc , uintptr sp , G *gp , TRecord *r ) 
{ 
int32 n; 
#line 480 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
n = runtime·gentraceback ( ( uintptr ) pc , ( uintptr ) sp , 0 , gp , 0 , r->stk , nelem ( r->stk ) , nil , nil , false ) ; 
if ( n < nelem ( r->stk ) ) 
r->stk[n] = 0; 
} 
void
runtime·GoroutineProfile(Slice b, intgo n, bool ok)
{
#line 485 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"

	uintptr pc, sp;
	TRecord *r;
	G *gp;
	
	sp = runtime·getcallersp(&b);
	pc = (uintptr)runtime·getcallerpc(&b);
	
	ok = false;
	n = runtime·gcount();
	if(n <= b.len) {
		runtime·semacquire(&runtime·worldsema, false);
		m->gcing = 1;
		runtime·stoptheworld();

		n = runtime·gcount();
		if(n <= b.len) {
			ok = true;
			r = (TRecord*)b.array;
			saveg(pc, sp, g, r++);
			for(gp = runtime·allg; gp != nil; gp = gp->alllink) {
				if(gp == g || gp->status == Gdead)
					continue;
				saveg(gp->sched.pc, gp->sched.sp, gp, r++);
			}
		}
	
		m->gcing = 0;
		runtime·semrelease(&runtime·worldsema);
		runtime·starttheworld();
	}
	FLUSH(&n);
	FLUSH(&ok);
}

#line 518 "/tmp/bindist375750859/go/src/pkg/runtime/mprof.goc"
void 
runtime·mprofinit ( void ) 
{ 
addrhash = runtime·persistentalloc ( ( 1<<AddrHashBits ) *sizeof *addrhash , 0 , &mstats.buckhash_sys ) ; 
} 