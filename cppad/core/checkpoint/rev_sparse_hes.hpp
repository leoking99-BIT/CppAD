# ifndef CPPAD_CORE_CHECKPOINT_REV_SPARSE_HES_HPP
# define CPPAD_CORE_CHECKPOINT_REV_SPARSE_HES_HPP

/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Eclipse Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */

namespace CppAD { // BEGIN_CPPAD_NAMESPACE

# if ! CPPAD_MULTI_THREAD_TMB
template <class Base>
template <class sparsity_type>
bool checkpoint<Base>::rev_sparse_hes(
	const vector<bool>&                     vx ,
	const vector<bool>&                     s  ,
	      vector<bool>&                     t  ,
	size_t                                  q  ,
	const sparsity_type&                    r  ,
	const sparsity_type&                    u  ,
	      sparsity_type&                    v  ,
	const vector<Base>&                     x  )
{	size_t n = f_.Domain();
# ifndef NDEBUG
	size_t m = f_.Range();
# endif
	CPPAD_ASSERT_UNKNOWN( vx.size() == n );
	CPPAD_ASSERT_UNKNOWN(  s.size() == m );
	CPPAD_ASSERT_UNKNOWN(  t.size() == n );
	CPPAD_ASSERT_UNKNOWN(  r.size() == n * q );
	CPPAD_ASSERT_UNKNOWN(  u.size() == m * q );
	CPPAD_ASSERT_UNKNOWN(  v.size() == n * q );
	//
	bool ok        = true;

	// make sure hes_sparse_bool_ has been set
	if( hes_sparse_bool_.size() == 0 )
		set_hes_sparse_bool();
	if( hes_sparse_set_.n_set() != 0 )
		hes_sparse_set_.resize(0, 0);
	CPPAD_ASSERT_UNKNOWN( hes_sparse_bool_.size() == n * n );
	CPPAD_ASSERT_UNKNOWN( hes_sparse_set_.n_set() == 0 );


	// compute sparsity pattern for T(x) = S(x) * f'(x)
	t = f_.RevSparseJac(1, s);
# ifndef NDEBUG
	for(size_t j = 0; j < n; j++)
		CPPAD_ASSERT_UNKNOWN( vx[j] || ! t[j] )
# endif

	// V(x) = f'(x)^T * g''(y) * f'(x) * R  +  g'(y) * f''(x) * R
	// U(x) = g''(y) * f'(x) * R
	// S(x) = g'(y)

	// compute sparsity pattern for A(x) = f'(x)^T * U(x)
	bool transpose = true;
	sparsity_type a(n * q);
	a = f_.RevSparseJac(q, u, transpose);

	// Need sparsity pattern for H(x) = (S(x) * f(x))''(x) * R,
	// but use less efficient sparsity for  f(x)''(x) * R so that
	// hes_sparse_set_ can be used every time this is needed.
	for(size_t i = 0; i < n; i++)
	{	for(size_t k = 0; k < q; k++)
		{	// initialize sparsity pattern for H(i,k)
			bool h_ik = false;
			// H(i,k) = sum_j f''(i,j) * R(j,k)
			for(size_t j = 0; j < n; j++)
			{	bool f_ij = hes_sparse_bool_[i * n + j];
				bool r_jk = r[j * q + k];
				h_ik     |= ( f_ij & r_jk );
			}
			// sparsity for H(i,k)
			v[i * q + k] = h_ik;
		}
	}

	// compute sparsity pattern for V(x) = A(x) + H(x)
	for(size_t i = 0; i < n; i++)
	{	for(size_t k = 0; k < q; k++)
			// v[ i * q + k ] |= a[ i * q + k];
			v[ i * q + k ] = bool(v[ i * q + k]) | bool(a[ i * q + k]);
	}
	return ok;
}
template <class Base>
bool checkpoint<Base>::rev_sparse_hes(
	const vector<bool>&                     vx ,
	const vector<bool>&                     s  ,
	      vector<bool>&                     t  ,
	size_t                                  q  ,
	const vectorBool&                       r  ,
	const vectorBool&                       u  ,
	      vectorBool&                       v  ,
	const vector<Base>&                     x  )
{	return rev_sparse_hes< vectorBool >(vx, s, t, q, r, u, v, x);
}
template <class Base>
bool checkpoint<Base>::rev_sparse_hes(
	const vector<bool>&                     vx ,
	const vector<bool>&                     s  ,
	      vector<bool>&                     t  ,
	size_t                                  q  ,
	const vector<bool>&                     r  ,
	const vector<bool>&                     u  ,
	      vector<bool>&                     v  ,
	const vector<Base>&                     x  )
{	return rev_sparse_hes< vector<bool> >(vx, s, t, q, r, u, v, x);
}
template <class Base>
bool checkpoint<Base>::rev_sparse_hes(
	const vector<bool>&                     vx ,
	const vector<bool>&                     s  ,
	      vector<bool>&                     t  ,
	size_t                                  q  ,
	const vector< std::set<size_t> >&       r  ,
	const vector< std::set<size_t> >&       u  ,
	      vector< std::set<size_t> >&       v  ,
	const vector<Base>&                     x  )
{	size_t n = f_.Domain();
# ifndef NDEBUG
	size_t m = f_.Range();
# endif
	CPPAD_ASSERT_UNKNOWN( vx.size() == n );
	CPPAD_ASSERT_UNKNOWN(  s.size() == m );
	CPPAD_ASSERT_UNKNOWN(  t.size() == n );
	CPPAD_ASSERT_UNKNOWN(  r.size() == n );
	CPPAD_ASSERT_UNKNOWN(  u.size() == m );
	CPPAD_ASSERT_UNKNOWN(  v.size() == n );
	//
	bool ok        = true;

	// make sure hes_sparse_set_ has been set
	if( hes_sparse_bool_.size() != 0 )
		hes_sparse_bool_.clear();
	if( hes_sparse_set_.n_set() == 0 )
		set_hes_sparse_set();
	CPPAD_ASSERT_UNKNOWN( hes_sparse_bool_.size() == 0 );
	CPPAD_ASSERT_UNKNOWN( hes_sparse_set_.n_set() == n );
	CPPAD_ASSERT_UNKNOWN( hes_sparse_set_.end()   == n );

	// compute sparsity pattern for T(x) = S(x) * f'(x)
	t = f_.RevSparseJac(1, s);
# ifndef NDEBUG
	for(size_t j = 0; j < n; j++)
		CPPAD_ASSERT_UNKNOWN( vx[j] || ! t[j] )
# endif

	// V(x) = f'(x)^T * g''(y) * f'(x) * R  +  g'(y) * f''(x) * R
	// U(x) = g''(y) * f'(x) * R
	// S(x) = g'(y)

	// compute sparsity pattern for A(x) = f'(x)^T * U(x)
	// 2DO: change a to use INTERNAL_SPARSE_SET
	bool transpose = true;
	vector< std::set<size_t> > a(n);
	a = f_.RevSparseJac(q, u, transpose);

	// Need sparsity pattern for H(x) = (S(x) * f(x))''(x) * R,
	// but use less efficient sparsity for  f(x)''(x) * R so that
	// hes_sparse_set_ can be used every time this is needed.
	for(size_t i = 0; i < n; i++)
	{	v[i].clear();
		local::sparse_list::const_iterator set_itr(
			hes_sparse_set_, i
		);
		size_t j = *set_itr;
		while( j < n )
		{	std::set<size_t>::const_iterator itr_j;
			const std::set<size_t>& r_j( r[j] );
			for(itr_j = r_j.begin(); itr_j != r_j.end(); itr_j++)
			{	size_t k = *itr_j;
				v[i].insert(k);
			}
			j = *(++set_itr);
		}
	}
	// compute sparsity pattern for V(x) = A(x) + H(x)
	std::set<size_t>::const_iterator itr;
	for(size_t i = 0; i < n; i++)
	{	for(itr = a[i].begin(); itr != a[i].end(); itr++)
		{	size_t j = *itr;
			CPPAD_ASSERT_UNKNOWN( j < q );
			v[i].insert(j);
		}
	}

	return ok;
}
# else // CPPAD_MULTI_THREAD_TMB
# define THREAD omp_get_thread_num()
template <class Base>
template <class sparsity_type>
bool checkpoint<Base>::rev_sparse_hes(
	const vector<bool>&                     vx ,
	const vector<bool>&                     s  ,
	      vector<bool>&                     t  ,
	size_t                                  q  ,
	const sparsity_type&                    r  ,
	const sparsity_type&                    u  ,
	      sparsity_type&                    v  ,
	const vector<Base>&                     x  )
{	size_t n = f_[THREAD].Domain();
# ifndef NDEBUG
	size_t m = f_[THREAD].Range();
# endif
	CPPAD_ASSERT_UNKNOWN( vx.size() == n );
	CPPAD_ASSERT_UNKNOWN(  s.size() == m );
	CPPAD_ASSERT_UNKNOWN(  t.size() == n );
	CPPAD_ASSERT_UNKNOWN(  r.size() == n * q );
	CPPAD_ASSERT_UNKNOWN(  u.size() == m * q );
	CPPAD_ASSERT_UNKNOWN(  v.size() == n * q );
	//
	bool ok        = true;

	// make sure hes_sparse_bool_ has been set
	if( hes_sparse_bool_[THREAD].size() == 0 )
		set_hes_sparse_bool();
	if( hes_sparse_set_[THREAD].n_set() != 0 )
		hes_sparse_set_[THREAD].resize(0, 0);
	CPPAD_ASSERT_UNKNOWN( hes_sparse_bool_[THREAD].size() == n * n );
	CPPAD_ASSERT_UNKNOWN( hes_sparse_set_[THREAD].n_set() == 0 );


	// compute sparsity pattern for T(x) = S(x) * f'(x)
	t = f_[THREAD].RevSparseJac(1, s);
# ifndef NDEBUG
	for(size_t j = 0; j < n; j++)
		CPPAD_ASSERT_UNKNOWN( vx[j] || ! t[j] )
# endif

	// V(x) = f'(x)^T * g''(y) * f'(x) * R  +  g'(y) * f''(x) * R
	// U(x) = g''(y) * f'(x) * R
	// S(x) = g'(y)

	// compute sparsity pattern for A(x) = f'(x)^T * U(x)
	bool transpose = true;
	sparsity_type a(n * q);
	a = f_[THREAD].RevSparseJac(q, u, transpose);

	// Need sparsity pattern for H(x) = (S(x) * f(x))''(x) * R,
	// but use less efficient sparsity for  f(x)''(x) * R so that
	// hes_sparse_set_ can be used every time this is needed.
	for(size_t i = 0; i < n; i++)
	{	for(size_t k = 0; k < q; k++)
		{	// initialize sparsity pattern for H(i,k)
			bool h_ik = false;
			// H(i,k) = sum_j f''(i,j) * R(j,k)
			for(size_t j = 0; j < n; j++)
			{	bool f_ij = hes_sparse_bool_[THREAD][i * n + j];
				bool r_jk = r[j * q + k];
				h_ik     |= ( f_ij & r_jk );
			}
			// sparsity for H(i,k)
			v[i * q + k] = h_ik;
		}
	}

	// compute sparsity pattern for V(x) = A(x) + H(x)
	for(size_t i = 0; i < n; i++)
	{	for(size_t k = 0; k < q; k++)
			// v[ i * q + k ] |= a[ i * q + k];
			v[ i * q + k ] = bool(v[ i * q + k]) | bool(a[ i * q + k]);
	}
	return ok;
}
template <class Base>
bool checkpoint<Base>::rev_sparse_hes(
	const vector<bool>&                     vx ,
	const vector<bool>&                     s  ,
	      vector<bool>&                     t  ,
	size_t                                  q  ,
	const vectorBool&                       r  ,
	const vectorBool&                       u  ,
	      vectorBool&                       v  ,
	const vector<Base>&                     x  )
{	return rev_sparse_hes< vectorBool >(vx, s, t, q, r, u, v, x);
}
template <class Base>
bool checkpoint<Base>::rev_sparse_hes(
	const vector<bool>&                     vx ,
	const vector<bool>&                     s  ,
	      vector<bool>&                     t  ,
	size_t                                  q  ,
	const vector<bool>&                     r  ,
	const vector<bool>&                     u  ,
	      vector<bool>&                     v  ,
	const vector<Base>&                     x  )
{	return rev_sparse_hes< vector<bool> >(vx, s, t, q, r, u, v, x);
}
template <class Base>
bool checkpoint<Base>::rev_sparse_hes(
	const vector<bool>&                     vx ,
	const vector<bool>&                     s  ,
	      vector<bool>&                     t  ,
	size_t                                  q  ,
	const vector< std::set<size_t> >&       r  ,
	const vector< std::set<size_t> >&       u  ,
	      vector< std::set<size_t> >&       v  ,
	const vector<Base>&                     x  )
{	size_t n = f_[THREAD].Domain();
# ifndef NDEBUG
	size_t m = f_[THREAD].Range();
# endif
	CPPAD_ASSERT_UNKNOWN( vx.size() == n );
	CPPAD_ASSERT_UNKNOWN(  s.size() == m );
	CPPAD_ASSERT_UNKNOWN(  t.size() == n );
	CPPAD_ASSERT_UNKNOWN(  r.size() == n );
	CPPAD_ASSERT_UNKNOWN(  u.size() == m );
	CPPAD_ASSERT_UNKNOWN(  v.size() == n );
	//
	bool ok        = true;

	// make sure hes_sparse_set_ has been set
	if( hes_sparse_bool_[THREAD].size() != 0 )
		hes_sparse_bool_[THREAD].clear();
	if( hes_sparse_set_[THREAD].n_set() == 0 )
		set_hes_sparse_set();
	CPPAD_ASSERT_UNKNOWN( hes_sparse_bool_[THREAD].size() == 0 );
	CPPAD_ASSERT_UNKNOWN( hes_sparse_set_[THREAD].n_set() == n );
	CPPAD_ASSERT_UNKNOWN( hes_sparse_set_[THREAD].end()   == n );

	// compute sparsity pattern for T(x) = S(x) * f'(x)
	t = f_[THREAD].RevSparseJac(1, s);
# ifndef NDEBUG
	for(size_t j = 0; j < n; j++)
		CPPAD_ASSERT_UNKNOWN( vx[j] || ! t[j] )
# endif

	// V(x) = f'(x)^T * g''(y) * f'(x) * R  +  g'(y) * f''(x) * R
	// U(x) = g''(y) * f'(x) * R
	// S(x) = g'(y)

	// compute sparsity pattern for A(x) = f'(x)^T * U(x)
	// 2DO: change a to use INTERNAL_SPARSE_SET
	bool transpose = true;
	vector< std::set<size_t> > a(n);
	a = f_[THREAD].RevSparseJac(q, u, transpose);

	// Need sparsity pattern for H(x) = (S(x) * f(x))''(x) * R,
	// but use less efficient sparsity for  f(x)''(x) * R so that
	// hes_sparse_set_ can be used every time this is needed.
	for(size_t i = 0; i < n; i++)
	{	v[i].clear();
		local::sparse_list::const_iterator set_itr(
			hes_sparse_set_[THREAD], i
		);
		size_t j = *set_itr;
		while( j < n )
		{	std::set<size_t>::const_iterator itr_j;
			const std::set<size_t>& r_j( r[j] );
			for(itr_j = r_j.begin(); itr_j != r_j.end(); itr_j++)
			{	size_t k = *itr_j;
				v[i].insert(k);
			}
			j = *(++set_itr);
		}
	}
	// compute sparsity pattern for V(x) = A(x) + H(x)
	std::set<size_t>::const_iterator itr;
	for(size_t i = 0; i < n; i++)
	{	for(itr = a[i].begin(); itr != a[i].end(); itr++)
		{	size_t j = *itr;
			CPPAD_ASSERT_UNKNOWN( j < q );
			v[i].insert(j);
		}
	}

	return ok;
}
# undef THREAD
# endif //  CPPAD_MULTI_THREAD_TMB

} // END_CPPAD_NAMESPACE
# endif
