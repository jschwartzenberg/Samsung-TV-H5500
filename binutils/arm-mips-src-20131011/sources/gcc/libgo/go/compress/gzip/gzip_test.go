// Copyright 2010 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package gzip

import (
	"bufio"
	"bytes"
	"io/ioutil"
	"testing"
	"time"
)

// TestEmpty tests that an empty payload still forms a valid GZIP stream.
func TestEmpty(t *testing.T) {
	buf := new(bytes.Buffer)

	if err := NewWriter(buf).Close(); err != nil {
		t.Fatalf("Writer.Close: %v", err)
	}

	r, err := NewReader(buf)
	if err != nil {
		t.Fatalf("NewReader: %v", err)
	}
	b, err := ioutil.ReadAll(r)
	if err != nil {
		t.Fatalf("ReadAll: %v", err)
	}
	if len(b) != 0 {
		t.Fatalf("got %d bytes, want 0", len(b))
	}
	if err := r.Close(); err != nil {
		t.Fatalf("Reader.Close: %v", err)
	}
}

// TestRoundTrip tests that gzipping and then gunzipping is the identity
// function.
func TestRoundTrip(t *testing.T) {
	buf := new(bytes.Buffer)

	w := NewWriter(buf)
	w.Comment = "comment"
	w.Extra = []byte("extra")
	w.ModTime = time.Unix(1e8, 0)
	w.Name = "name"
	if _, err := w.Write([]byte("payload")); err != nil {
		t.Fatalf("Write: %v", err)
	}
	if err := w.Close(); err != nil {
		t.Fatalf("Writer.Close: %v", err)
	}

	r, err := NewReader(buf)
	if err != nil {
		t.Fatalf("NewReader: %v", err)
	}
	b, err := ioutil.ReadAll(r)
	if err != nil {
		t.Fatalf("ReadAll: %v", err)
	}
	if string(b) != "payload" {
		t.Fatalf("payload is %q, want %q", string(b), "payload")
	}
	if r.Comment != "comment" {
		t.Fatalf("comment is %q, want %q", r.Comment, "comment")
	}
	if string(r.Extra) != "extra" {
		t.Fatalf("extra is %q, want %q", r.Extra, "extra")
	}
	if r.ModTime.Unix() != 1e8 {
		t.Fatalf("mtime is %d, want %d", r.ModTime.Unix(), uint32(1e8))
	}
	if r.Name != "name" {
		t.Fatalf("name is %q, want %q", r.Name, "name")
	}
	if err := r.Close(); err != nil {
		t.Fatalf("Reader.Close: %v", err)
	}
}

// TestLatin1 tests the internal functions for converting to and from Latin-1.
func TestLatin1(t *testing.T) {
	latin1 := []byte{0xc4, 'u', 0xdf, 'e', 'r', 'u', 'n', 'g', 0}
	utf8 := "??u??erung"
	z := Reader{r: bufio.NewReader(bytes.NewBuffer(latin1))}
	s, err := z.readString()
	if err != nil {
		t.Fatalf("readString: %v", err)
	}
	if s != utf8 {
		t.Fatalf("read latin-1: got %q, want %q", s, utf8)
	}

	buf := bytes.NewBuffer(make([]byte, 0, len(latin1)))
	c := Writer{w: buf}
	if err = c.writeString(utf8); err != nil {
		t.Fatalf("writeString: %v", err)
	}
	s = buf.String()
	if s != string(latin1) {
		t.Fatalf("write utf-8: got %q, want %q", s, string(latin1))
	}
}

// TestLatin1RoundTrip tests that metadata that is representable in Latin-1
// survives a round trip.
func TestLatin1RoundTrip(t *testing.T) {
	testCases := []struct {
		name string
		ok   bool
	}{
		{"", true},
		{"ASCII is OK", true},
		{"unless it contains a NUL\x00", false},
		{"no matter where \x00 occurs", false},
		{"\x00\x00\x00", false},
		{"L??tin-1 also passes (U+00E1)", true},
		{"but L??tin Extended-A (U+0100) does not", false},
		{"neither does ?????????", false},
		{"invalid UTF-8 also \xffails", false},
		{"\x00 as does L??tin-1 with NUL", false},
	}
	for _, tc := range testCases {
		buf := new(bytes.Buffer)

		w := NewWriter(buf)
		w.Name = tc.name
		err := w.Close()
		if (err == nil) != tc.ok {
			t.Errorf("Writer.Close: name = %q, err = %v", tc.name, err)
			continue
		}
		if !tc.ok {
			continue
		}

		r, err := NewReader(buf)
		if err != nil {
			t.Errorf("NewReader: %v", err)
			continue
		}
		_, err = ioutil.ReadAll(r)
		if err != nil {
			t.Errorf("ReadAll: %v", err)
			continue
		}
		if r.Name != tc.name {
			t.Errorf("name is %q, want %q", r.Name, tc.name)
			continue
		}
		if err := r.Close(); err != nil {
			t.Errorf("Reader.Close: %v", err)
			continue
		}
	}
}
