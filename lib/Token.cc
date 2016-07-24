/* Token.cc: Token passed from tokenizer to parser
 * Copyright 2014, 2015, 2016 Vincent Damewood
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

// Visual Studio complains about strcpy without this.
#if defined _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif /* _MSC_VER */

#include <cstring>

#include "Token.h"

namespace Silikego
{
	class Token::State
	{
	public:
		State(TokenType NewType) : Type(NewType) {}
		State(int NewValue) : Type(INTEGER), Integer(NewValue) {}
		State(double NewValue) : Type(FLOAT), Float(NewValue) {}
		State(const char *NewId) : Type(ID), Id(NewId) {}

		TokenType Type;
		union
		{
			int Integer;
			double Float;
		};
		std::string Id;
	};

	Token::Token(TokenType NewType) : S(new State(NewType)) { }
	Token::Token(int NewValue) : S(new State(NewValue)) { }
	Token::Token(double NewValue) : S(new State(NewValue)) { }
	Token::Token(const char *NewId) : S(new State(NewId)) { }

	Token::~Token()
	{
		delete S;
	}

	Token::TokenType Token::Type() const
	{
		return S->Type;
	}

	int Token::Integer() const
	{
		return S->Integer;
	}

	float Token::Float() const
	{
		return S->Float;
	}

	const char *Token::Id() const
	{
		return S->Id.c_str();
	}
}