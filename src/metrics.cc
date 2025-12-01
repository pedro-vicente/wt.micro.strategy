#include "metrics.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// PROFITABILITY METRICS
//
// Profitability ratios measure a company's ability to generate earnings relative to revenue,
// operating costs, balance sheet assets, and shareholders' equity. These ratios show how well
// a company uses its assets to produce profit and create value for shareholders.
//
// Reference: Brigham & Houston, "Fundamentals of Financial Management", Chapter 4
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// gross_profit
//
// Gross Profit = Revenue - Cost of Goods Sold (COGS)
//
// Gross profit represents the profit a company makes after deducting the direct costs
// associated with producing and selling its products or services. COGS includes direct
// labor, materials, and manufacturing overhead.
//
// This metric shows how efficiently a company uses labor and supplies in the production
// process. A higher gross profit indicates better efficiency in converting raw materials
// and labor into revenue.
//
// Reference: FASB ASC 225-10 (Income Statement Presentation)
// Reference: https://www.investopedia.com/terms/g/grossprofit.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::gross_profit() const
{
  return revenue - cogs;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// gross_margin
//
// Gross Margin = Gross Profit / Revenue
//
// Also known as Gross Profit Margin or Gross Profit Percentage. This ratio expresses
// gross profit as a percentage of revenue, showing what portion of each dollar of
// revenue is retained after accounting for direct production costs.
// 
// Reference: https://www.investopedia.com/terms/g/gross_profit_margin.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::gross_margin() const
{
  return (revenue > 0) ? gross_profit() / revenue : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ebitda
//
// EBITDA = Revenue - COGS - Operating Expenses + Depreciation + Amortization
//        = Operating Income + Depreciation + Amortization
//
// Earnings Before Interest, Taxes, Depreciation, and Amortization (EBITDA) is a measure
// of a company's overall financial performance. It approximates operating cash flow by
// excluding non-cash expenses (D&A) and financing/tax effects.
//
// Reference: Damodaran, A. "Investment Valuation" (Wiley)
// Reference: https://www.investopedia.com/terms/e/ebitda.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::ebitda() const
{
  return revenue - cogs - operating_expenses + depreciation + amortization;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ebit
//
// EBIT = EBITDA - Depreciation - Amortization
//      = Revenue - COGS - Operating Expenses
//      = Operating Income
//
// Earnings Before Interest and Taxes (EBIT), also called Operating Income or Operating
// Profit, measures profit from core business operations before financing costs and taxes.
//
// Reference: https://www.investopedia.com/terms/e/ebit.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::ebit() const
{
  return ebitda() - depreciation - amortization;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// net_income
//
// Net Income = EBIT - Interest Expense - Taxes
//            = Revenue - All Expenses
//
// Net Income (also called Net Profit, Net Earnings, or "the bottom line") represents
// the total profit remaining after all expenses, including operating costs, interest,
// taxes, and any other charges.
//
// Reference: FASB ASC 220 (Comprehensive Income)
// Reference: https://www.investopedia.com/terms/n/netincome.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::net_income() const
{
  return ebit() - interest - taxes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// net_margin
//
// Net Margin = Net Income / Revenue
//
// Also called Net Profit Margin or Profit Margin. This ratio shows what percentage of
// revenue translates into profit after all expenses. It is the most comprehensive
// profitability metric as it accounts for all costs.
//
// Reference: https://www.investopedia.com/terms/n/net_margin.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::net_margin() const
{
  return (revenue > 0) ? net_income() / revenue : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// operating_margin
//
// Operating Margin = EBIT / Revenue = Operating Income / Revenue
//
// Operating margin measures the percentage of revenue remaining after paying for
// variable costs of production (COGS) and fixed costs (operating expenses like
// rent, utilities, salaries). It excludes interest and taxes.
//
// Reference: https://www.investopedia.com/terms/o/operatingmargin.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::operating_margin() const
{
  return (revenue > 0) ? ebit() / revenue : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// LIQUIDITY METRICS
//
// Liquidity ratios measure a company's ability to meet short-term obligations using
// its most liquid assets. These ratios are critical for assessing financial health
// and the risk of bankruptcy or financial distress.
//
// Creditors and suppliers use these ratios to assess creditworthiness.
// Investors use them to evaluate financial stability and risk.
//
// Reference: Ross, Westerfield & Jordan, "Corporate Finance", Chapter 3
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// working_capital
//
// Working Capital = Current Assets - Current Liabilities
//
// Working capital (also called Net Working Capital or NWC) represents the operating
// liquidity available to a business. It measures the company's short-term financial
// health and operational efficiency.
// 
// Reference: https://www.investopedia.com/terms/w/workingcapital.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::working_capital() const
{
  return current_assets - current_liabilities;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// current_ratio
//
// Current Ratio = Current Assets / Current Liabilities
//
// The current ratio measures a company's ability to pay short-term obligations
// (those due within one year). It indicates how many dollars of current assets
// are available to cover each dollar of current liabilities.
//
// Reference: https://www.investopedia.com/terms/c/currentratio.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::current_ratio() const
{
  return (current_liabilities > 0) ? current_assets / current_liabilities : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// quick_ratio
//
// Quick Ratio = (Current Assets - Inventory) / Current Liabilities
//
// Also called the Acid-Test Ratio. This is a more stringent measure of liquidity
// than the current ratio because it excludes inventory, which may not be easily
// converted to cash (especially for companies with slow-moving or specialized goods).
//
// Reference: https://www.investopedia.com/terms/q/quickratio.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::quick_ratio() const
{
  return (current_liabilities > 0) ? (current_assets - inventory) / current_liabilities : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// LEVERAGE METRICS
//
// Leverage (or solvency) ratios measure the degree to which a company finances its
// operations with debt versus equity. These ratios indicate long-term financial risk
// and the company's ability to meet long-term obligations.
//
// Reference: Brigham & Houston, "Fundamentals of Financial Management", Chapter 4
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// equity
//
// Equity = Total Assets - Total Liabilities
//        = Shareholders' Equity
//        = Book Value
//
// Equity represents the residual interest in the assets of an entity after deducting
// liabilities. It is the amount that would theoretically be distributed to shareholders
// if all assets were liquidated and all debts paid.
//
// Reference: FASB ASC 505 (Equity)
// Reference: https://www.investopedia.com/terms/s/shareholdersequity.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::equity() const
{
  return total_assets - total_liabilities;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// debt_to_equity
//
// Debt-to-Equity Ratio = Total Liabilities / Shareholders' Equity
//
// This ratio compares a company's total debt to its shareholder equity. It indicates
// the relative proportion of debt and equity used to finance the company's assets.
//
// Reference: https://www.investopedia.com/terms/d/debtequityratio.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::debt_to_equity() const
{
  return (equity() > 0) ? total_liabilities / equity() : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// debt_ratio
//
// Debt Ratio = Total Liabilities / Total Assets
//
// Also called the Total Debt Ratio. This ratio measures the proportion of a company's
// assets that are financed by debt (creditors) rather than equity (shareholders).
//
// Mathematical relationship: Debt Ratio + Equity Ratio = 1.0
//
//
// Reference: https://www.investopedia.com/terms/d/debtratio.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::debt_ratio() const
{
  return (total_assets > 0) ? total_liabilities / total_assets : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// RETURN METRICS
//
// Return metrics measure how effectively a company generates profits from its
// resources (assets and equity). These ratios are essential for evaluating
// management effectiveness and comparing investment alternatives.
//
// The DuPont Analysis framework decomposes ROE into:
//   ROE = Net Margin x Asset Turnover x Financial Leverage
//       = (Net Income/Revenue) x (Revenue/Assets) x (Assets/Equity)
//
// Reference: CFA Institute, "Financial Reporting and Analysis"
/////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////
// return_on_assets
//
// Return on Assets (ROA) = Net Income / Total Assets
//
// ROA measures how efficiently a company uses its assets to generate profit.
// It answers the question: "How much profit does each dollar of assets produce?"
//
// ROA can be decomposed (DuPont Analysis):
//   ROA = Net Margin x Asset Turnover
//       = (Net Income/Revenue) x (Revenue/Total Assets)
//
// Reference: https://www.investopedia.com/terms/r/returnonassets.asp
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::return_on_assets() const
{
  return (total_assets > 0) ? net_income() / total_assets : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// return_on_equity
//
// Return on Equity (ROE) = Net Income / Shareholders' Equity
//
// ROE measures the profit generated for each dollar of shareholders' equity.
// It is often considered the most important profitability metric for investors
// because it directly relates to shareholder returns.
//
// DuPont Analysis (3-factor):
//   ROE = Net Margin x Asset Turnover x Equity Multiplier
//       = (Net Income/Revenue) x (Revenue/Assets) x (Assets/Equity)
//
// DuPont Analysis (5-factor) for deeper analysis:
//   ROE = Tax Burden x Interest Burden x EBIT Margin x Asset Turnover x Leverage
//
//
// Reference: https://www.investopedia.com/terms/r/returnonequity.asp
// Reference: Palepu, K.G. & Healy, P.M. "Business Analysis and Valuation"
/////////////////////////////////////////////////////////////////////////////////////////////////////

double FinancialMetrics::return_on_equity() const
{
  return (equity() > 0) ? net_income() / equity() : 0;
}

