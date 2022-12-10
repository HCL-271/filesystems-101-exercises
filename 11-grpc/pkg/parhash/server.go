package parhash

import (
	"context"

	"net"
	"sync"
	"log"
	"github.com/pkg/errors"
	"google.golang.org/grpc"

	hashpb "fs101ex/pkg/gen/hashsvc"
	parhashpb "fs101ex/pkg/gen/parhashsvc"
	"golang.org/x/sync/semaphore"
	"fs101ex/pkg/workgroup"
	//zero iq copy from hash variant
)

type Config struct {
	ListenAddr   string
	BackendAddrs []string
	Concurrency  int
}

// Implement a server that responds to ParallelHash()
// as declared in /proto/parhash.proto.
//
// The implementation of ParallelHash() must not hash the content
// of buffers on its own. Instead, it must send buffers to backends
// to compute hashes. Buffers must be fanned out to backends in the
// round-robin fashion.
//
// For example, suppose that 2 backends are configured and ParallelHash()
// is called to compute hashes of 5 buffers. In this case it may assign
// buffers to backends in this way:
//
//	backend 0: buffers 0, 2, and 4,
//	backend 1: buffers 1 and 3.
//
// Requests to hash individual buffers must be issued concurrently.
// Goroutines that issue them must run within /pkg/workgroup/Wg. The
// concurrency within workgroups must be limited by Server.sem.
//
// WARNING: requests to ParallelHash() may be concurrent, too.
// Make sure that the round-robin fanout works in that case too,
// and evenly distributes the load across backends.
type Server struct {
	conf Config
	MutexSyncronizer sync.Mutex
	checker int


	stop context.CancelFunc
	l    net.Listener
	wg   sync.WaitGroup
	
	sem *semaphore.Weighted
}

func New(conf Config) *Server {
	return &Server{
		
		conf: conf,
		sem:  semaphore.NewWeighted(int64(conf.Concurrency)),
		checker: 0,
		
	}
}

func (s *Server) Start(ctx context.Context) (err error) {
	defer func() { err = errors.Wrapf(err, "Start()") }()

	ctx, s.stop = context.WithCancel(ctx)

	s.l, err = net.Listen("tcp", s.conf.ListenAddr)
	if err != nil {
		return err
	}

	srv := grpc.NewServer()
	parhashpb.RegisterParallelHashSvcServer(srv, s)
	//change method to parallel hash

	s.wg.Add(2)
	go func() {
		defer s.wg.Done()

		srv.Serve(s.l)
	}()
	go func() {
		defer s.wg.Done()

		<-ctx.Done()
		s.l.Close()
	}()

	return nil
	
}

func (s *Server) ListenAddr() string {
	return s.l.Addr().String()
}

func (s *Server) Stop() {
	s.stop()
	s.wg.Wait()
}

func (s *Server) ParallelHash(ctx context.Context, req *parhashpb.ParHashReq) (resp *parhashpb.ParHashResp, err error) {
	
	clients := make([]hashpb.HashSvcClient, len(s.conf.BackendAddrs))
	joins := make([]*grpc.ClientConn, len(s.conf.BackendAddrs))
	for i, addr := range s.conf.BackendAddrs {
		joins[i], err = grpc.Dial(addr, grpc.WithInsecure())
		if err != nil {
			log.Fatalf("failed to connect to %q: %v", s.conf.BackendAddrs[i], err)
		}
		defer joins[i].Close()
		clients[i] = hashpb.NewHashSvcClient(joins[i])
	}
	var (
		workgroup1     = workgroup.New(workgroup.Config{Sem: s.sem})
		hashes = make([][]byte, len(req.Data))
	)
	for i := range req.Data {
		number := i
		workgroup1.Go(ctx, func(ctx context.Context) error {
			s.MutexSyncronizer.Lock()
			previous := s.checker
			s.checker += 1
			if s.checker >= len(s.conf.BackendAddrs) {
				s.checker = 0
			}
			s.MutexSyncronizer.Unlock()
			resp, err := clients[previous].Hash(ctx, &hashpb.HashReq{Data: req.Data[number]})
			if err != nil {
				return err
			}
			s.MutexSyncronizer.Lock()
			hashes[number] = resp.Hash
			s.MutexSyncronizer.Unlock()
			return nil
		})
	}
	if err := workgroup1.Wait(); err != nil {
		log.Fatalf("failed to hash data: %v", err)
	}
	return &parhashpb.ParHashResp{Hashes: hashes}, nil
}
