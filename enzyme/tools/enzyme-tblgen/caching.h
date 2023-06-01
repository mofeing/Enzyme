// clang-format off
//

using namespace llvm;
void emit_mat_caching(TGPattern &pattern, raw_ostream &os) {

  const auto argUsers = pattern.getArgUsers();
  const auto actArgs = pattern.getActiveArgs();
  const auto typeMap = pattern.getArgTypeMap();
  const auto nameVec = pattern.getArgNames();

  for (size_t i = 0; i < nameVec.size(); i++) {
    if (typeMap.lookup(i) != argType::mldData) 
      continue;
    assert(typeMap.lookup(i+1) == argType::mldLD);
    const auto matName = nameVec[i];
    const auto matPosition = i;
    const auto matUsers = argUsers.lookup(matPosition);
    const auto ldName = nameVec[i + 1];
    const auto ldPosition = i + 1;
    const auto ldUsers = argUsers.lookup(ldPosition);
    if (matUsers.size() == 0) {
os << "  bool cache_" << matName << " = false;\n";
    } else {
      os 
<< "  bool cache_" << matName
<< "  = (cacheMode &&\n"
<< "          uncacheable_" << matName;
      for (size_t user: matUsers) {
        auto name = nameVec[user];
        if (name == matName)
          continue; // adjoint of x won't need x
        os 
<< " && active_" << name;
      }
      os 
<< ");\n";
    }
//    os
//<< "  bool cache_" << ldName << " = true;\n";
//<< "  bool cache_" << ldName << " = false;\n";
      // xinc is needed to be preserved if
      // 1) it is potentially overwritten AND EITHER
      //     a) x is active (for performing the shadow increment) or
      //     b) we're not caching x and need xinc to compute the
      //     derivative of a different variable
//      os 
//<< "  const bool need_" << ldName << " = (true || active_" << matName;
//<< "  const bool need_" << ldName << " = (active_" << matName;
//      if (ldUsers.size() > 0) {
//        os 
//<< "  || (!cache_" << matName << " && (";
//        bool first = true;
//        for (size_t user: ldUsers) {
//          auto name = nameVec[user];
//          os 
//<< ((first) ? "" : " || ") << "active_" << name;
//          first = false;
//        }
//        os 
//<< "))";
//      }
//      os 
//<< ");\n"
//<< "  if (true) {\n"
////<< "  if (byRef && uncacheable_" << ldName << " && need_" << ldName << ") {\n"
//<< "    cacheTypes.push_back(intType);\n"
//<< "    cache_" << ldName << " = true;\n "
//<< "  }\n\n";
  }
}

void emit_vec_caching(TGPattern &pattern, raw_ostream &os) {

  const auto argUsers = pattern.getArgUsers();
  const auto actArgs = pattern.getActiveArgs();
  const auto typeMap = pattern.getArgTypeMap();
  const auto nameVec = pattern.getArgNames();

  for (size_t i = 0; i < nameVec.size(); i++) {
    if (typeMap.lookup(i) != argType::vincData) 
      continue;
    assert(typeMap.lookup(i+1) == argType::vincInc);
    const auto vecName = nameVec[i];
    const auto vecPosition = i;
    const auto vecUsers = argUsers.lookup(vecPosition);
    const auto incName = nameVec[i + 1];
    const auto incPosition = i + 1;
    const auto incUsers = argUsers.lookup(incPosition);
      os 
<< "  bool cache_" << vecName
<< "  = (cacheMode &&\n"
<< "          uncacheable_" << vecName;
      for (size_t user: vecUsers) {
        auto name = nameVec[user];
        if (name == vecName)
          continue; // adjoint of x won't need x
        os 
<< " && active_" << name;
      }
      os 
<< ");\n";
//<< "  bool cache_" << incName << " = true;\n";
//<< "  bool cache_" << incName << " = false;\n";
      // xinc is needed to be preserved if
      // 1) it is potentially overwritten AND EITHER
      //     a) x is active (for performing the shadow increment) or
      //     b) we're not caching x and need xinc to compute the
      //     derivative of a different variable
//      os 
//<< "  const bool need_" << incName << " = (true || active_" << vecName;
//<< "  const bool need_" << incName << " = (active_" << vecName;
//      if (incUsers.size() > 0) {
//        os 
//<< "  || (!cache_" << vecName << " && (";
//        bool first = true;
//        for (size_t user: incUsers) {
//          auto name = nameVec[user];
//          os 
//<< ((first) ? "" : " || ") << "active_" << name;
//          first = false;
//        }
//        os 
//<< "))";
//      }
//      os 
//<< ");\n"
//<< "  if (true) {\n"
////<< "  if (byRef && uncacheable_" << incName << " && need_" << incName << ") {\n"
//<< "    cacheTypes.push_back(intType);\n"
//<< "    cache_" << incName << " = true;\n "
//<< "  }\n\n";

  }
}

void emit_scalar_caching(TGPattern &pattern, raw_ostream &os) {
  auto actArgs = pattern.getActiveArgs();
  auto typeMap = pattern.getArgTypeMap();
  auto nameVec = pattern.getArgNames();
  os 
<< "  // len, fp must be preserved if overwritten\n";
  for (size_t i = 0; i < nameVec.size(); i++) {
    auto typeOfArg = typeMap.lookup(i);
    if (typeOfArg == argType::len || typeOfArg == argType::fp || typeOfArg == argType::trans 
        || typeOfArg == argType::vincInc || typeOfArg == argType::mldLD) {
      auto scalarType = "";
      if (typeOfArg == len || typeOfArg == vincInc || typeOfArg == mldLD) {
        scalarType = "intType";
      } else if (typeOfArg == trans) {
        scalarType = "charType";
      } else if (typeOfArg == fp) {
        scalarType = "fpType";
      }
      auto name = nameVec[i];
      os 
<< "  bool cache_" << name << " = true;\n"
<< "  const bool need_" << name << " = true;\n"
//<< "  bool cache_" << name << " = false;\n"
<< "  if (true) {\n"
//<< "  if (byRef && uncacheable_" << name << ") {\n";
<< "    cacheTypes.push_back(" << scalarType << ");\n"
<< "    cache_" << name << " = true;\n"
<< "  }\n";
    }
  }
}

void emit_scal_cacheValues(std::string argName, std::string scalType, raw_ostream &os) {
  os
<< "        addValueToCache(arg_" << argName <<", cache_" << argName <<", " << scalType << ", cacheValues, BuilderZ);\n";
}

void emit_cache_for_reverse(TGPattern &pattern, raw_ostream &os) {
  auto actArgs = pattern.getActiveArgs();
  auto typeMap = pattern.getArgTypeMap();
  auto nameVec = pattern.getArgNames();
  auto argUsers = pattern.getArgUsers();
  //auto primalName = pattern.getName();

  os 
<< "  if ((Mode == DerivativeMode::ReverseModeCombined ||\n"
<< "       Mode == DerivativeMode::ReverseModePrimal) && cachetype) {\n"
<< "    SmallVector<Value *, 2> cacheValues;\n";
 
  os << "    if (byRef) {\n";
  for (size_t i = 0; i < nameVec.size(); i++) {
    auto typeOfArg = typeMap.lookup(i);
    auto name = nameVec[i];

    if (typeOfArg == argType::len) {
      emit_scal_cacheValues(name, "intType", os);
    } else if (typeOfArg == vincInc || typeOfArg == mldLD) {
      emit_scal_cacheValues(name, "intType", os);
    } else if (typeOfArg == trans) {
      emit_scal_cacheValues(name, "charType", os);
    } else if (typeOfArg == argType::fp) {
      emit_scal_cacheValues(name, "fpType", os);
    }
  }
  os << "    }\n";

  for (size_t i = 0; i < actArgs.size(); i++) {
    size_t argIdx = actArgs[i];
    auto typeOfArg = typeMap.lookup(argIdx);
    if (typeOfArg != argType::vincData)
      continue;
    assert(typeMap.lookup(argIdx+1) == argType::vincInc);
    auto vecName = nameVec[argIdx];
    auto incName = nameVec[argIdx+1];
    // TODO: remove last hardcoded len_n usages to support blas lv2/3 
    os
<< "    if (cache_" << vecName << ") {\n"
<< "      auto malins = CreateAllocation(BuilderZ, fpType, arg_n);\n"
<< "      Value *arg = BuilderZ.CreateBitCast(malins, castvals[" << i << "]);\n"
<< "      Function *dmemcpy;\n"
<< "      if (EnzymeBlasCopy) {\n"
<< "        ValueType valueTypes[] = {";
    { bool comma = false;
    for (auto i : nameVec) {
      if (comma) os << ", ";
      os << "ValueType::None";
      comma = true;
    }
    os << "};\n";
os <<
   "         valueTypes[" << argIdx << "] = ValueType::Primal;\n"
<< "         if (byRef) valueTypes[" << argIdx+1 << "] = ValueType::Primal;\n";
    }
    for (auto len_pos : pattern.getRelatedLengthArgs(argIdx) ) {
os << "         if (byRef) valueTypes[" << len_pos << "] = ValueType::Primal;\n";
    }
os << "        dmemcpy = getOrInsertMemcpyStridedBlas(*gutils->oldFunc->getParent(), cast<PointerType>(castvals[" << i << "]),\n"
<< "            intType, blas, julia_decl);\n"
<< "        Value *args[5] = {arg_n, arg_" << vecName << ", arg_" << incName << ", arg, to_blas_callconv(BuilderZ, ConstantInt::get(intType, 1), byRef, julia_decl_type, allocationBuilder)};\n"
<< "        if (julia_decl)\n"
<< "          args[3] = BuilderZ.CreatePtrToInt(args[3], type_" << vecName << ");\n"
<< "        BuilderZ.CreateCall(dmemcpy, args,\n"
<< "            gutils->getInvertedBundles(&call, valueTypes,\n"
<< "            BuilderZ, /*lookup*/ false));\n"
<< "      } else {\n"
<< "        ValueType valueTypes[] = {";
    { bool comma = false;
    for (auto i : nameVec) {
      if (comma) os << ", ";
      os << "ValueType::None";
      comma = true;
    }
    os << "};\n";
os <<
   "         valueTypes[" << argIdx << "] = ValueType::Primal;\n"
<< "         if (byRef) valueTypes[" << argIdx+1 << "] = ValueType::Primal;\n";
    }
    for (auto len_pos : pattern.getRelatedLengthArgs(argIdx) ) {
os << "         if (byRef) valueTypes[" << len_pos << "] = ValueType::Primal;\n";
    }
os << "        dmemcpy = getOrInsertMemcpyStrided(*gutils->oldFunc->getParent(), cast<PointerType>(castvals[" << i << "]),\n"
<< "            intType, 0, 0);\n"
<< "        Value *args[4] = {arg, arg_" << vecName << ", arg_n, arg_" << incName << "};\n"
<< "        if (julia_decl)\n"
<< "          args[1] = BuilderZ.CreateIntToPtr(args[1], castvals[" << i << "]);\n"
<< "        BuilderZ.CreateCall(dmemcpy, args,\n"
<< "            gutils->getInvertedBundles(&call, valueTypes,\n"
<< "            BuilderZ, /*lookup*/ false));\n"
<< "      }\n"
<< "      cacheValues.push_back(arg);\n"
<< "    }\n";
  }
  // TODO: same as above, but not for vincData, but for mldData
  for (size_t i = 0; i < actArgs.size(); i++) {
    size_t argIdx = actArgs[i];
    auto typeOfArg = typeMap.lookup(argIdx);
    if (typeOfArg != argType::mldData)
      continue;
    assert(typeMap.lookup(argIdx+1) == argType::mldLD);
    auto matName = nameVec[argIdx];
    auto ldName = nameVec[argIdx+1];
    auto dimensions = pattern.getRelatedLengthArgs(argIdx);
    assert(dimensions.size() == 2);
    assert(typeMap.lookup(dimensions[0]) == argType::len);
    assert(typeMap.lookup(dimensions[1]) == argType::len);
    std::string dim1 = "arg_" + nameVec[dimensions[0]];
    std::string dim2 = "arg_" + nameVec[dimensions[1]];
    // 
    // TODO: remove last hardcoded len_n usages to support blas lv2/3 
    os
<< "    if (cache_" << matName << ") {\n"
<< "      Value *matSize;\n"
<< "      auto charType = IntegerType::get(intType->getContext(), 8);\n"
<< "      auto *M = " << dim1 << ";\n"
<< "      auto *N = " << dim2 << ";\n"
<< "        matSize = BuilderZ.CreateMul(M,N);\n"
<< "      if (byRef) {\n"
// next two probably are needed?
//<< "        auto MP = BuilderZ.CreatePointerCast(M, PointerType::get(intType, cast<PointerType>(M->getType())->getAddressSpace()));\n"
//<< "        auto NP = BuilderZ.CreatePointerCast(N, PointerType::get(intType, cast<PointerType>(N->getType())->getAddressSpace()));\n"
//<< "        auto len1 = BuilderZ.CreateLoad(intType, MP);\n"
//<< "        auto len2 = BuilderZ.CreateLoad(intType, NP);\n"
//<< "        matSize = BuilderZ.CreateMul(len1, len2);\n"
<< "      } else {\n"
<< "        matSize = BuilderZ.CreateMul(M,N);\n"
<< "      }\n"
<< "      auto malins = CreateAllocation(BuilderZ, fpType, matSize);\n"
<< "      Value *arg = BuilderZ.CreateBitCast(malins, castvals[" << i << "]);\n"
<< "      Function *dmemcpy;\n"
<< "      assert(EnzymeLapackCopy);\n"
<< "      {\n"
<< "        ValueType valueTypes[] = {";
    { bool comma = false;
    for (auto i : nameVec) {
      if (comma) os << ", ";
      os << "ValueType::None";
      comma = true;
    }
    os << "};\n";
os <<
   "         valueTypes[" << argIdx << "] = ValueType::Primal;\n"
<< "         if (byRef) valueTypes[" << argIdx+1 << "] = ValueType::Primal;\n";
    }
    for (auto len_pos : dimensions ) {
os << "         if (byRef) valueTypes[" << len_pos << "] = ValueType::Primal;\n";
    }
os << "        dmemcpy = getOrInsertMemcpyStridedLapack(*gutils->oldFunc->getParent(), cast<PointerType>(castvals[" << i << "]),\n"
<< "            intType, blas, julia_decl);\n"
<< "        Value *uplo = llvm::ConstantInt::get(charType, 0);\n" // garbage data, just should not match U or L
<< "        uplo = to_blas_callconv(BuilderZ, uplo, byRef, julia_decl_type, allocationBuilder);\n"
<< "        Value *newLD = to_blas_callconv(BuilderZ, arg_" << ldName << ", byRef, julia_decl_type, allocationBuilder);\n"
<< "        Value *newM = to_blas_callconv(BuilderZ, N, byRef, julia_decl_type, allocationBuilder);\n"
<< "        Value *newN = to_blas_callconv(BuilderZ, M, byRef, julia_decl_type, allocationBuilder);\n"
<< "        Value *args[7] = {uplo, newM, newN, arg_" << matName << ", newLD, arg, newM};\n"
<< "        if (julia_decl) {\n"
<< "          args[3] = BuilderZ.CreatePtrToInt(args[3], type_" << matName << ");\n"
<< "          args[5] = BuilderZ.CreatePtrToInt(args[5], type_" << matName << ");\n"
<< "        }\n"
<< "        BuilderZ.CreateCall(dmemcpy, args,\n"
<< "            gutils->getInvertedBundles(&call, valueTypes,\n"
<< "            BuilderZ, /*lookup*/ false));\n"
<< "      }\n"
<< "      cacheValues.push_back(arg);\n"
<< "    }\n";
  }

  os
<< "    if (cacheValues.size() == 1) {\n"
<< "      cacheval = cacheValues[0];\n"
<< "    } else {\n"
<< "      cacheval = UndefValue::get(cachetype);\n"
<< "      for (auto tup : llvm::enumerate(cacheValues))\n"
<< "        cacheval = BuilderZ.CreateInsertValue(cacheval, tup.value(), tup.index());\n"
<< "    }\n"
<< "    gutils->cacheForReverse(BuilderZ, cacheval,\n"
<< "                            getIndex(&call, CacheType::Tape));\n"
<< "  }\n"
<< "  unsigned cacheidx = 0;\n";
 
  for (size_t i = 0; i < nameVec.size(); i++) {
    auto name = nameVec[i];
    auto typeOfArg = typeMap.lookup(i);
    if (typeOfArg == argType::vincData) {
      assert(typeMap.lookup(i+1) == argType::vincInc);
      auto vecName = nameVec[i];
      auto incName = nameVec[i+1];
      os 
<< "  Value *true_" << incName << " = arg_" << incName << ";\n"
<< "  Value *" << incName << " = true_" << incName << ";\n"
<< "  Value *data_" << vecName << " = arg_" << vecName << ";\n"
<< "  Value *data_ptr_" << vecName << " = nullptr;\n";
    } else if (typeOfArg == argType::mldData) {
      assert(typeMap.lookup(i+1) == argType::mldLD);
      auto vecName = nameVec[i];
      auto ldName = nameVec[i+1];
      os 
<< "  Value *true_" << ldName << " = arg_" << ldName << ";\n"
<< "  Value *" << ldName << " = true_" << ldName << ";\n"
<< "  Value *data_" << vecName << " = arg_" << vecName << ";\n"
<< "  Value *data_ptr_" << vecName << " = nullptr;\n";
    } else if (typeOfArg == argType::len) {
os<< "  Value *" << name << " = arg_" << name << ";\n";
    } else if (typeOfArg == argType::fp) {
      os
<< "  Value *fp_" << name << " = arg_" << name << ";\n"; 
    } else if (typeOfArg == argType::trans) {
      os
<< "  Value *true_" << name << " = arg_" << name << ";\n"
<< "  Value *" << name << " = true_" << name << ";\n";
    }
  }


  os
<< "  IRBuilder<> Builder2(call.getParent());\n"
<< "  switch (Mode) {\n"
<< "    case DerivativeMode::ReverseModeCombined:\n"
<< "    case DerivativeMode::ReverseModeGradient:\n"
<< "      getReverseBuilder(Builder2);\n"
<< "      break;\n"
<< "    case DerivativeMode::ForwardMode:\n"
<< "    case DerivativeMode::ForwardModeSplit:\n"
<< "      Builder2.SetInsertPoint(BuilderZ.GetInsertBlock(),\n"
<< "                              BuilderZ.GetInsertPoint());\n"
<< "      Builder2.setFastMathFlags(getFast());\n"
<< "      break;\n"
<< "    case DerivativeMode::ReverseModePrimal:\n"
<< "      break;\n"
<< "  }\n\n";
}

void emit_caching(TGPattern &pattern, raw_ostream &os) {

  auto actArgs = pattern.getActiveArgs();
  auto nameVec = pattern.getArgNames();

  // 1. No caching for fwd-mode
  // 2. Deactivate caching for uncacheable_args
  // 3. Only caching if we do need the primary for an active gradient.
  // 4. (New) Cache vec if it is overwritten but the input vec is required.
  os 
<< "  SmallVector<Type *, 2> cacheTypes;\n\n";

  emit_scalar_caching(pattern, os);
  emit_mat_caching(pattern, os);
  emit_vec_caching(pattern, os);

  for (auto actEn : llvm::enumerate(actArgs)) {
    auto name = nameVec[actEn.value()];
    os 
<< "  if (cache_" << name << ")\n"
<< "    cacheTypes.push_back(castvals[" << actEn.index() << "]);\n";
  }
  os
<< "  Type *cachetype = nullptr;\n"
<< "  switch (cacheTypes.size()) {\n"
<< "  case 0:\n"
<< "    break;\n"
<< "  case 1:\n"
<< "    cachetype = cacheTypes[0];\n"
<< "    break;\n"
<< "  default:\n"
<< "    cachetype = StructType::get(call.getContext(), cacheTypes);\n"
<< "    break;\n"
<< "  }\n\n";

  emit_cache_for_reverse(pattern, os);
}


