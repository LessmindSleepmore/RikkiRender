#pragma once

template <class t>
t max(t a, t b) {
	return a > b ? a : b;
}

template <class t>
t min(t a, t b) {
	return a > b ? b : a;
}