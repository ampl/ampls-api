using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cplexsharp
{
  public static class Utils
  {
    static bool IsNumericType(this object o)
    {
      switch (Type.GetTypeCode(o.GetType()))
      {
        case TypeCode.Byte:
        case TypeCode.SByte:
        case TypeCode.UInt16:
        case TypeCode.UInt32:
        case TypeCode.UInt64:
        case TypeCode.Int16:
        case TypeCode.Int32:
        case TypeCode.Int64:
        case TypeCode.Decimal:
        case TypeCode.Double:
        case TypeCode.Single:
          return true;
        default:
          return false;
      }
    }
    static string getAMPLRepr(object v)
    {
      if (v.IsNumericType())
        return v.ToString();
      else
        return string.Format("'{0}'", v.ToString());
    }
    public static string getAMPLVarName(params object[] tuple)
    {
      bool indexing = tuple.Length > 1;
      if (!indexing)
        return tuple[0].ToString();
      StringBuilder sb = new StringBuilder();
      sb.Append(tuple[0].ToString());
      sb.Append("[");
      for(int i=1; i<tuple.Length-1; i++)
      {
        sb.Append(getAMPLRepr(tuple[i]));
        sb.Append(", ");
      }
      sb.Append(getAMPLRepr(tuple[tuple.Length - 1]));
      sb.Append("]");
      return sb.ToString();
    }
  }
}
