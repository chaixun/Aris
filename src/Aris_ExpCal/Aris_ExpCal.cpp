﻿#include <sstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstring>
#include <algorithm>

using namespace std;

#include "Aris_ExpCal.h"
#include <Eigen/Eigen>

namespace Aris
{
	namespace DynKer
	{
		MATRIX::MATRIX()
			: m(0)
			, n(0)
			, isRowMajor(true)
			, pData(nullptr)
		{
		}
		MATRIX::MATRIX(const MATRIX &other)
			: m(other.m)
			, n(other.n)
			, isRowMajor(other.isRowMajor)
			, pData(m*n > 0 ? new double[m*n]: nullptr)
		{
			//memcpy(pData, other.pData, m*n*sizeof(double));
			std::copy_n(other.pData, m*n, pData);
		}
		MATRIX::MATRIX(MATRIX &&other)
			:MATRIX()
		{
			this->Swap(other);
		}
		MATRIX::MATRIX(int m, int n)
			: m(m)
			, n(n)
			, isRowMajor(true)
			, pData(m*n > 0 ? new double[m*n] : nullptr)
		{
		}
		MATRIX::MATRIX(int m, int n, const double *Data)
			: MATRIX(m,n)
		{
			if ((m*n>0) && (Data != nullptr))
				memcpy(pData, Data, m*n*sizeof(double));
		}

		MATRIX::MATRIX(double value)
			: m(1)
			, n(1)
			, isRowMajor(true)
			, pData(new double[1])
		{
			*pData = value;
		}
		MATRIX::MATRIX(const std::initializer_list<double> &Data)
			: MATRIX()
		{
			std::vector<MATRIX> A;
			for (auto i : Data)
			{
				A.push_back(i);
			}
			(*this) = Aris::DynKer::CombineRowMatrices<std::vector<MATRIX> >(A);
		}
		MATRIX::MATRIX(const std::initializer_list<std::vector<MATRIX> > &matrices)
			: MATRIX()
		{
			(*this) = Aris::DynKer::CombineMatrices<decltype(matrices)>(matrices);
		}

		MATRIX &MATRIX::operator=(MATRIX other)
		{
			this->Swap(other);
			return *this;
		}
		MATRIX::~MATRIX()
		{
			delete[]pData;
		}
		MATRIX &MATRIX::Swap(MATRIX &other)
		{
			std::swap(this->m, other.m);
			std::swap(this->n, other.n);
			std::swap(this->isRowMajor, other.isRowMajor);
			std::swap(this->pData, other.pData);

			return *this;
		}
		MATRIX operator + (const MATRIX &m1, const MATRIX &m2)
		{
			MATRIX ret;

			if ((m1.m == 1) && (m1.n == 1))
			{
				ret = MATRIX(m2);

				for (int i = 0; i < ret.Length(); ++i)
				{
					ret.pData[i] += m1(0, 0);
				}
			}
			else if ((m2.m == 1) && (m2.n == 1))
			{
				ret = MATRIX(m1);

				for (int i = 0; i < ret.Length(); ++i)
				{
					ret.pData[i] += m2(0, 0);
				}
			}
			else if ((m1.m == m2.m) && (m1.n == m2.n))
			{
				ret.Resize(m1.m, m1.n);

				for (int i = 0; i < ret.m; i++)
				{
					for (int j = 0; j < ret.n; j++)
					{
						ret(i, j) = m1(i, j) + m2(i, j);
					}
				}
			}
			else
			{
				throw std::logic_error("Can't plus matrices, the dimensions are not equal");
			}
			return ret;
		}
		MATRIX operator - (const MATRIX &m1, const MATRIX &m2)
		{
			MATRIX ret;

			if ((m1.m == 1) && (m1.n == 1))
			{
				ret = MATRIX(m2);

				for (int i = 0; i < ret.Length(); ++i)
				{
					ret.pData[i] = m1(0, 0) - ret.pData[i];
				}
			}
			else if ((m2.m == 1) && (m2.n == 1))
			{
				ret = MATRIX(m1);

				for (int i = 0; i < ret.Length(); ++i)
				{
					ret.pData[i] -= m2(0, 0);
				}
			}
			else if ((m1.m == m2.m) && (m1.n == m2.n))
			{
				ret.Resize(m1.m, m1.n);

				for (int i = 0; i < ret.m; i++)
				{
					for (int j = 0; j < ret.n; j++)
					{
						ret(i, j) = m1(i, j) - m2(i, j);
					}
				}
			}
			else
			{
				throw std::logic_error("Can't minus matrices, the dimensions are not equal");
			}

			return ret;
		}
		MATRIX operator * (const MATRIX &m1, const MATRIX &m2)
		{
			MATRIX ret;

			if ((m1.m == 1) && (m1.n == 1))
			{
				ret = MATRIX(m2);

				for (int i = 0; i < ret.Length(); ++i)
				{
					ret.pData[i] *= m1(0, 0);
				}
			}
			else if ((m2.m == 1) && (m2.n == 1))
			{
				ret = MATRIX(m1);

				for (int i = 0; i < ret.Length(); ++i)
				{
					ret.pData[i] *= m2(0, 0);
				}
			}
			else if (m1.n == m2.m)
			{
				ret.Resize(m1.m, m2.n);

				if (m1.isRowMajor)
				{
					if (m2.isRowMajor)
					{
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > m1Eigen(m1.pData, m1.m, m1.n);
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > m2Eigen(m2.pData, m2.m, m2.n);
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > retEigen(ret.pData, ret.m, ret.n);
						
						retEigen = m1Eigen*m2Eigen;
						/*cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
							m1.m, m2.n, m1.n,
							1, m1.Data(), m1.n, m2.Data(), m2.n,
							0, ret.Data(), ret.n);*/
					}
					else
					{
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > m1Eigen(m1.pData, m1.m, m1.n);
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> > m2Eigen(m2.pData, m2.m, m2.n);
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > retEigen(ret.pData, ret.m, ret.n);

						retEigen = m1Eigen*m2Eigen;
						
						/*cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans,
							m1.m, m2.n, m1.n,
							1, m1.Data(), m1.n, m2.Data(), m2.m,
							0, ret.Data(), ret.n);*/
					}
				}
				else
				{
					if (m2.isRowMajor)
					{
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> > m1Eigen(m1.pData, m1.m, m1.n);
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > m2Eigen(m2.pData, m2.m, m2.n);
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > retEigen(ret.pData, ret.m, ret.n);

						retEigen = m1Eigen*m2Eigen;
						
						
						/*cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans,
							m1.m, m2.n, m1.n,
							1, m1.Data(), m1.m, m2.Data(), m2.n,
							0, ret.Data(), ret.n);*/
					}
					else
					{
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> > m1Eigen(m1.pData, m1.m, m1.n);
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> > m2Eigen(m2.pData, m2.m, m2.n);
						Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> > retEigen(ret.pData, ret.m, ret.n);

						retEigen = m1Eigen*m2Eigen;
						
						/*cblas_dgemm(CblasRowMajor, CblasTrans, CblasTrans,
							m1.m, m2.n, m1.n,
							1, m1.Data(), m1.m, m2.Data(), m2.m,
							0, ret.Data(), ret.n);*/
					}
				}


			}
			else
			{
				throw std::logic_error("Can't multiply matrices, the dimensions are not equal");
			}

			return ret;
		}
		MATRIX operator / (const MATRIX &m1, const MATRIX &m2)
		{
			MATRIX ret;

			if ((m1.m == 1) && (m1.n == 1))
			{
				ret = MATRIX(m2);

				for (int i = 0; i < ret.Length(); ++i)
				{
					ret.pData[i] = m1(0, 0) / ret.pData[i];
				}
			}
			else if ((m2.m == 1) && (m2.n == 1))
			{
				ret = MATRIX(m1);

				for (int i = 0; i < ret.Length(); ++i)
				{
					ret.pData[i] /= m2(0, 0);
				}
			}
			else
			{
				throw std::logic_error("Right now, divide operator of matrices is not added");
			}

			return ret;
		}
		MATRIX operator - (const MATRIX &m1)
		{
			MATRIX m;
			m.Resize(m1.m, m1.n);

			for (int i = 0; i < m1.m*m1.n; i++)
			{
				m.pData[i] = -m1.pData[i];
			}
			return m;
		}
		MATRIX operator + (const MATRIX &m1)
		{
			return m1;
		}

		void MATRIX::Resize(int i, int j)
		{
			m = i;
			n = j;

			if (pData != nullptr)
				delete[]pData;

			pData = new double[i*j];
		}
		void MATRIX::CopySubMatrixTo(const MATRIX &subMat, int beginRow, int beginCol, int rowNum, int colNum)
		{
			if ((beginRow + subMat.m > m) || (beginCol + subMat.n > n))
			{
				throw std::logic_error("Function CopySubMatrixTo must have subMat smaller than self matrix");
			}

			for (int i = 0; i < subMat.m; ++i)
			{
				for (int j = 0; j < subMat.n; ++j)
				{
					this->operator()(i + beginRow, j + beginCol)=subMat(i,j);
				}
			}

		}
		std::string MATRIX::ToString() const
		{
			std::stringstream stream;
			
			stream.precision(15);
			stream << "{";
			for (int i = 0; i < m; ++i)
			{
				for (int j = 0; j < n; ++j)
				{
					stream << this->operator()(i, j);
					if (j<n - 1)
						stream << " , ";
				}
				if (i<m-1)
					stream << " ;\n";
			}
			stream << "}";

			return stream.str();
		};

		std::string SeperateString(const std::string &s)
		{
			static const std::string SEMICOLONrateOpr("+-*/^()[]{},;");
			std::string ret;

			for (const char &key : s)
			{
				/*判断是否为科学计数法的数字*/
				if ((&key > s.data() + 1) && (&key<s.data() +s.size() - 2))
				{
					if ((key == '+') || (key == '-'))
					{
						if ((*(&key - 1) == 'e') 
							&& (*(&key - 2) <= '9')
							&& (*(&key - 2) >= '0')
							&& (*(&key + 1) <= '9')
							&& (*(&key + 1) >= '0'))
						{
							ret += key;
							continue;
						}
					}
				}

				
				if (SEMICOLONrateOpr.find(key) != SEMICOLONrateOpr.npos)
				{
					ret = ret + " " + key + " ";
				}
				else
				{
					ret += key;
				}
			}
			return ret;
		};
		
		CALCULATOR::TOKENS CALCULATOR::Expression2Tokens(const std::string &expression)
		{
			TOKENS tokens;
			TOKEN token;
			
			std::string text = SeperateString(expression);

			std::stringstream stream(text);

			int id = 0;

			while (stream >> token.word)
			{
				token.id = id;
				id++;
				
				/*判断是否为逗号*/
				if (*token.word.c_str() == ',')
				{
					token.type = TOKEN::COMMA;
					tokens.push_back(token);
					continue;
				}
				/*判断是否为分号*/
				if (*token.word.c_str() == ';')
				{
					token.type = TOKEN::SEMICOLON;
					tokens.push_back(token);
					continue;
				}
				/*判断是否为左括号*/
				if (*token.word.c_str() == '(')
				{
					token.type = TOKEN::PARENTHESIS_L;
					tokens.push_back(token);
					continue;
				}
				/*判断是否为右括号*/
				if (*token.word.c_str() == ')')
				{
					token.type = TOKEN::PARENTHESIS_R;
					tokens.push_back(token);
					continue;
				}
				/*判断是否为左中括号*/
				if (*token.word.c_str() == '[')
				{
					token.type = TOKEN::BRACKET_L;
					tokens.push_back(token);
					continue;
				}
				/*判断是否为右中括号*/
				if (*token.word.c_str() == ']')
				{
					token.type = TOKEN::BRACKET_R;
					tokens.push_back(token);
					continue;
				}
				/*判断是否为左大括号*/
				if (*token.word.c_str() == '{')
				{
					token.type = TOKEN::BRACE_L;
					tokens.push_back(token);
					continue;
				}
				/*判断是否为右大括号*/
				if (*token.word.c_str() == '}')
				{
					token.type = TOKEN::BRACE_R;
					tokens.push_back(token);
					continue;
				}
				/*判断是否为数字*/
				std::stringstream stream(token.word);
				double num;
				if (stream >> num)
				{
					token.type = TOKEN::CONST_VALUE;
					token.cst = MATRIX(num);
					tokens.push_back(token);
					continue;
				}
				/*判断是否为操作符*/
				auto f = operators.find(token.word);
				if (operators.find(token.word) != operators.end())
				{
					token.type = TOKEN::OPERATOR;
					token.opr = &f->second;
					tokens.push_back(token);
					continue;
				}

				/*否则则可能为函数或者变量，这里先不做处理*/
				token.type = TOKEN::NO;
				
				tokens.push_back(token);
				continue;
			}

			for (auto i = tokens.begin(); i != tokens.end();++i)
			{
				if (i->type == TOKEN::NO)
				{
					/*识别下一个是否为括号，即函数开始符*/
					if (((i+1)!=tokens.end())&&((i + 1)->type == TOKEN::PARENTHESIS_L))
					{
						auto f = this->functions.find(i->word);
						if (f != functions.end())
						{
							i->type = TOKEN::FUNCTION;
							i->fun = &f->second;
						}
						else
						{
							std::string information = "unrecognized function:" + i->word;
							throw std::logic_error(information.c_str());
						}
					}
					else
					{
						auto f = variables.find(i->word);
						if (f != variables.end())
						{
							i->type = TOKEN::VARIABLE;
							i->var = &f->second;
						}
						else
						{
							std::string information = "unrecognized varible:" + i->word;
							throw std::logic_error(information.data());
						}
					}
				}
			}

			return tokens;
		}
		MATRIX CALCULATOR::CaculateTokens(TOKENS::iterator beginToken, TOKENS::iterator endToken)
		{
			if (beginToken >= endToken)
			{
				throw std::logic_error("invalid expression");
			}
			
			auto i = beginToken;

			MATRIX value;
			value.Resize(0, 0);

			bool isBegin = true;

			while (i < endToken)
			{
				/*如果没有当前值，证明刚刚开始计算*/
				if (isBegin)
				{
					isBegin = false;
					switch (i->type)
					{
					case TOKEN::PARENTHESIS_L:
						value = CaculateValueInParentheses(i, endToken);
						break;
					case TOKEN::BRACE_L:
						value = CaculateValueInBraces(i, endToken);
						break;
					case TOKEN::CONST_VALUE:
						value = i->cst;
						i++;
						break;
					case TOKEN::OPERATOR:
						value = CaculateValueInOperator(i, endToken);
						break;
					case TOKEN::VARIABLE:
						value = *i->var;
						i++;
						break;
					case TOKEN::FUNCTION:
						value = CaculateValueInFunction(i, endToken);
						break;
					default:
						throw std::logic_error("expression not valid");
					}
				}
				else//如果有当前值，但没有操作符
				{				
					if (i->type == TOKEN::OPERATOR)
					{
						if (i->opr->priority_ur)
						{
							value = i->opr->fun_ur(value);
							i++;
						}
						else if (i->opr->priority_b > 0)
						{
							auto e = FindNextEqualLessPrecedenceBinaryOpr(i + 1, endToken, i->opr->priority_b);
							value = i->opr->fun_b(value, CaculateTokens(i + 1, e));
							i = e;
						}
						else
						{
							throw std::logic_error("expression not valid");
						}
					}
					else
					{
						throw std::logic_error("expression not valid: lack operator");
					}
				}
			}



			return value;
		}

		MATRIX CALCULATOR::CaculateValueInParentheses(TOKENS::iterator &i, TOKENS::iterator maxEndToken)
		{
			auto beginPar = i+1;
			auto endPar = FindNextOutsideToken(i + 1, maxEndToken, TOKEN::PARENTHESIS_R);

			i = endPar + 1;

			return CaculateTokens(beginPar, endPar);
		}
		MATRIX CALCULATOR::CaculateValueInBraces(TOKENS::iterator &i, TOKENS::iterator maxEndToken)
		{
			std::vector<std::vector<MATRIX>> matrices;

			auto endBce = FindNextOutsideToken(i+1, maxEndToken, TOKEN::BRACE_R);
			matrices = GetMatrices(i+1, endBce);
			i = endBce+1;

			return CombineMatrices(matrices);
		}
		MATRIX CALCULATOR::CaculateValueInFunction(TOKENS::iterator &i, TOKENS::iterator maxEndToken)
		{
			auto beginPar = i + 1;
			auto endPar = FindNextOutsideToken(beginPar + 1, maxEndToken, TOKEN::PARENTHESIS_R);

			auto matrices = GetMatrices(beginPar + 1, endPar);
			
			if (matrices.size() != 1)
			{
				throw std::logic_error("some function in expression do not have valid inputs");
			}

			std::vector<MATRIX> params = matrices.front();

			auto f=i->fun->funs.find(params.size());
			if (f != i->fun->funs.end())
			{
				i = endPar + 1;
				return f->second(params);
			}
			else
			{
				std::string s = "function ",s1;
				s += i->word + " do not has vesion with ";
				std::stringstream ss;
				ss << params.size();
				ss >> s1;
				s+=s1+ " parameters";
				throw std::logic_error(s.c_str());
			}
		}
		MATRIX CALCULATOR::CaculateValueInOperator(TOKENS::iterator &i, TOKENS::iterator maxEndToken)
		{
			auto opr = i;
			i = FindNextEqualLessPrecedenceBinaryOpr(opr + 1, maxEndToken, opr->opr->priority_ul);
			return opr->opr->fun_ul(CaculateTokens(opr + 1, i));
		}
		
		CALCULATOR::TOKENS::iterator CALCULATOR::FindNextOutsideToken(TOKENS::iterator beginToken, TOKENS::iterator endToken, TOKEN::TYPE type)
		{
			int parNum = 0;
			int braNum = 0;
			int bceNum = 0;

			auto nextPlace = beginToken;

			while (nextPlace < endToken)
			{
				if ((parNum == 0) && (braNum == 0) && (bceNum == 0))
				{
					if (nextPlace->type == type)
					{
						return nextPlace;
					}
				}
				
				switch (nextPlace->type)
				{
				case TOKEN::PARENTHESIS_L:
					parNum++;
					break;
				case TOKEN::PARENTHESIS_R:
					parNum--;
					break;
				case TOKEN::BRACKET_L:
					braNum++;
					break;
				case TOKEN::BRACKET_R:
					braNum--;
					break;
				case TOKEN::BRACE_L:
					bceNum++;
					break;
				case TOKEN::BRACE_R:
					bceNum--;
					break;
				default:
					break;
				}

				nextPlace++;
			}

			return nextPlace;
		}
		CALCULATOR::TOKENS::iterator CALCULATOR::FindNextEqualLessPrecedenceBinaryOpr(TOKENS::iterator beginToken, TOKENS::iterator endToken, int precedence)
		{
			auto nextOpr= beginToken;

			while (nextOpr < endToken)
			{
				nextOpr = FindNextOutsideToken(nextOpr, endToken, TOKEN::OPERATOR);

				if ((nextOpr==endToken) || (nextOpr->opr->priority_b <= precedence))
				{
					break;
				}
				else
				{
					nextOpr++;
				}
			}

			return nextOpr;
		}
		std::vector<std::vector<MATRIX> > CALCULATOR::GetMatrices(TOKENS::iterator beginToken, TOKENS::iterator endToken)
		{
			std::vector<std::vector<MATRIX> > ret;
			
			auto rowBegin = beginToken;
			while (rowBegin < endToken)
			{
				auto rowEnd = FindNextOutsideToken(rowBegin, endToken, TOKEN::SEMICOLON);
				auto colBegin = rowBegin;

				ret.push_back(std::vector<MATRIX>());

				while (colBegin < rowEnd)
				{
					auto colEnd = FindNextOutsideToken(colBegin, rowEnd, TOKEN::COMMA);

					ret.back().push_back(CaculateTokens(colBegin, colEnd));

					if (colEnd == endToken)
						colBegin = colEnd;
					else
						colBegin = colEnd + 1;
				}


				if (rowEnd == endToken)
					rowBegin = rowEnd;
				else
					rowBegin = rowEnd+1;
			}


			return ret;
		}


		MATRIX CALCULATOR::CalculateExpression(const std::string &expression)
		{
			this->tokens=Expression2Tokens(expression);
			return CaculateTokens(tokens.begin(), tokens.end());
		}
		void CALCULATOR::AddVariable(const std::string &name, const MATRIX &value)
		{
			if (variables.find(name) != variables.end())
			{
				throw std::logic_error("variable already exists");
			}
			variables.insert(make_pair(name, value));
		}


	}

}

