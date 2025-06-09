/*
 * Copyright (c) 2025 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/**
 * @file definitions.h
 * @brief Definitions of types and constants for Turing Machines
 *
 * @details The definitions below follow how we treated Turing Machines in our course of "Informatica Teorica" at "University of Florence (UniFI)"
 * Turing Machines we have worked with have a finite number of states, and the alphabet they are based on have a finite number of symbols, so they
 * can be numerated and represented through an index (eg. 10 states => From q0 to q9, so 0 to 9)
 * Only for this reason the blank symbol is represented with '-1', since the 'first' symbol of any alphabet will be 0
 */

/// Namespace that holds all the definitions and classes used for the execution of Turing Machines
namespace mdt {

    /// A state can be represented through its index, only non-negative indexes are considered correct
    using state = int;

    /// A symbol can be represented through its index, only non-negative indexes are considered correct
    using symbol = int;

    /// Since symbols are non-negative, the blank symbol, a 'special', non-existing, symbol gets to be -1
    constexpr symbol blank = -1;

}

#endif