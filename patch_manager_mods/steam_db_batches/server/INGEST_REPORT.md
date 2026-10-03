# Server ingest report

Generated 2026-10-03 01:42.

## (a) Steam: trusted server names vs name_map and branch DB

### functions vs name_map (25 trusted)

agree 0 / conflict 7 / server-only 18 (skipped 0 default-named)

| addr | server | other | detail | server source |
|---|---|---|---|---|
| 0x00407920 | WinMain | Global::WinMain | conf=medium | USER_DEFINED |
| 0x00409550 | lpStartAddress_00409550 | "Knights Cinematic Backdrop" thread | conf=medium | USER_DEFINED |
| 0x0040d430 | lpStartAddress_0040d430 | CExoMoviePlayer thread proc | conf=high | USER_DEFINED |
| 0x00434350 | CreateProcessedTexture | GLRender::CreateProcessedTexture | conf=medium | USER_DEFINED |
| 0x0070caa0 | CExoStreamingSoundSourceInternal__InitializeSource | StreamingSource::OpenStream | conf=medium | USER_DEFINED |
| 0x00722a30 | CResWave__OnResourceServiced | CResWave::OnResourceServiced (vtbl, speculative slot) | conf=medium | USER_DEFINED |
| 0x0073d060 | lpStartAddress_0073d060 | AsyncIoThreadProc | conf=high | USER_DEFINED |

<details><summary>server-only (first 18 of 18)</summary>

| addr | server | source |
|---|---|---|
| 0x00427280 | AurTextureSetCubeMapParameter | USER_DEFINED |
| 0x00433cd0 | CreateTexture | USER_DEFINED |
| 0x004670b0 | GobOrderCmp | USER_DEFINED |
| 0x00467640 | DoLightSaberBucket | USER_DEFINED |
| 0x00467770 | Gob::VisibilityCheck | USER_DEFINED |
| 0x00467d70 | DoGobBuckets | USER_DEFINED |
| 0x004682c0 | DoHologramGobBuckets | USER_DEFINED |
| 0x004686e0 | DoBlurBuckets | USER_DEFINED |
| 0x00474700 | Post | USER_DEFINED |
| 0x0070bbb0 | CExoSound__GetBestEAXAvailable | USER_DEFINED |
| 0x007112f0 | lpStartAddress_007112f0 | USER_DEFINED |
| 0x007366d0 | CExoBaseInternal__CreateResourceExtensionTable | USER_DEFINED |
| 0x007dacb0 | CSWGuiOptionsSoundAdvanced__CSWGuiOptionsSoundAdvanced | USER_DEFINED |
| 0x007dbaa0 | CSWGuiOptionsSoundAdvanced__OnEAXLeft | USER_DEFINED |
| 0x007dbb10 | CSWGuiOptionsSoundAdvanced__OnEAXRight | USER_DEFINED |
| 0x007dbd60 | CSWGuiOptionsSoundAdvanced__OnForceSoftware | USER_DEFINED |
| 0x007dbf80 | CSWGuiOptionsSoundAdvanced__SetUpOptions | USER_DEFINED |
| 0x0091d5a2 | entry | IMPORTED |

</details>

### functions vs branch DB (25 trusted)

agree 0 / conflict 0 / server-only 25 (skipped 0 default-named)

<details><summary>server-only (first 25 of 25)</summary>

| addr | server | source |
|---|---|---|
| 0x00407920 | WinMain | USER_DEFINED |
| 0x00409550 | lpStartAddress_00409550 | USER_DEFINED |
| 0x0040d430 | lpStartAddress_0040d430 | USER_DEFINED |
| 0x00427280 | AurTextureSetCubeMapParameter | USER_DEFINED |
| 0x00433cd0 | CreateTexture | USER_DEFINED |
| 0x00434350 | CreateProcessedTexture | USER_DEFINED |
| 0x004670b0 | GobOrderCmp | USER_DEFINED |
| 0x00467640 | DoLightSaberBucket | USER_DEFINED |
| 0x00467770 | Gob::VisibilityCheck | USER_DEFINED |
| 0x00467d70 | DoGobBuckets | USER_DEFINED |
| 0x004682c0 | DoHologramGobBuckets | USER_DEFINED |
| 0x004686e0 | DoBlurBuckets | USER_DEFINED |
| 0x00474700 | Post | USER_DEFINED |
| 0x0070bbb0 | CExoSound__GetBestEAXAvailable | USER_DEFINED |
| 0x0070caa0 | CExoStreamingSoundSourceInternal__InitializeSource | USER_DEFINED |
| 0x007112f0 | lpStartAddress_007112f0 | USER_DEFINED |
| 0x00722a30 | CResWave__OnResourceServiced | USER_DEFINED |
| 0x007366d0 | CExoBaseInternal__CreateResourceExtensionTable | USER_DEFINED |
| 0x0073d060 | lpStartAddress_0073d060 | USER_DEFINED |
| 0x007dacb0 | CSWGuiOptionsSoundAdvanced__CSWGuiOptionsSoundAdvanced | USER_DEFINED |
| 0x007dbaa0 | CSWGuiOptionsSoundAdvanced__OnEAXLeft | USER_DEFINED |
| 0x007dbb10 | CSWGuiOptionsSoundAdvanced__OnEAXRight | USER_DEFINED |
| 0x007dbd60 | CSWGuiOptionsSoundAdvanced__OnForceSoftware | USER_DEFINED |
| 0x007dbf80 | CSWGuiOptionsSoundAdvanced__SetUpOptions | USER_DEFINED |
| 0x0091d5a2 | entry | IMPORTED |

</details>

### symbols vs name_map (3227 trusted)

agree 0 / conflict 0 / server-only 3227 (skipped 0 default-named)

<details><summary>server-only (first 200 of 3227)</summary>

| addr | server | source |
|---|---|---|
| 0x004010bd | override::prt_4010bd_8d2134c3 | USER_DEFINED |
| 0x00407461 | override::prt_407461_f3434af | USER_DEFINED |
| 0x004077b3 | override::prt_4077b3_f3434af | USER_DEFINED |
| 0x004077e3 | override::prt_4077e3_f3434af | USER_DEFINED |
| 0x004078c3 | override::prt_4078c3_ea24a0a | USER_DEFINED |
| 0x00408395 | override::prt_408395_33cfa187 | USER_DEFINED |
| 0x004253c8 | override::prt_4253c8_407e704c | USER_DEFINED |
| 0x0042546b | override::prt_42546b_407e704c | USER_DEFINED |
| 0x00428a24 | override::prt_428a24_8d2134c3 | USER_DEFINED |
| 0x00428dba | override::prt_428dba_8d2134c3 | USER_DEFINED |
| 0x0042902e | override::prt_42902e_558b9120 | USER_DEFINED |
| 0x00458fc5 | override::prt_458fc5_f3434af | USER_DEFINED |
| 0x0045900e | override::prt_45900e_407e704c | USER_DEFINED |
| 0x00460c67 | override::prt_460c67_7de13c66 | USER_DEFINED |
| 0x00460caf | override::prt_460caf_9907cd9f | USER_DEFINED |
| 0x00460e97 | override::prt_460e97_7de13c66 | USER_DEFINED |
| 0x00460edf | override::prt_460edf_9907cd9f | USER_DEFINED |
| 0x0046393b | override::prt_46393b_c448bc6a | USER_DEFINED |
| 0x004646f9 | override::prt_4646f9_f3434af | USER_DEFINED |
| 0x00464742 | override::prt_464742_407e704c | USER_DEFINED |
| 0x00464772 | override::prt_464772_8d2134c3 | USER_DEFINED |
| 0x0046f680 | override::prt_46f680_f3434af | USER_DEFINED |
| 0x0046fc47 | override::prt_46fc47_dbe336c3 | USER_DEFINED |
| 0x0046fcb0 | override::prt_46fcb0_f3434af | USER_DEFINED |
| 0x0046fd6b | override::prt_46fd6b_aee7b64a | USER_DEFINED |
| 0x0046fe01 | override::prt_46fe01_50dddcd6 | USER_DEFINED |
| 0x0046fe7a | override::prt_46fe7a_7de13c66 | USER_DEFINED |
| 0x004700b0 | override::prt_4700b0_ebf76999 | USER_DEFINED |
| 0x00470173 | override::prt_470173_ebf76999 | USER_DEFINED |
| 0x0047050f | override::prt_47050f_f3434af | USER_DEFINED |
| 0x00470df8 | override::prt_470df8_e40233a0 | USER_DEFINED |
| 0x00474a8a | override::prt_474a8a_aee7b64a | USER_DEFINED |
| 0x00474b94 | override::prt_474b94_8d2134c3 | USER_DEFINED |
| 0x00474ee6 | override::prt_474ee6_b7eeacb5 | USER_DEFINED |
| 0x004751f4 | override::prt_4751f4_f3434af | USER_DEFINED |
| 0x00475632 | override::prt_475632_f3434af | USER_DEFINED |
| 0x00475b7b | override::prt_475b7b_4bfc86ce | USER_DEFINED |
| 0x00476468 | override::prt_476468_f3434af | USER_DEFINED |
| 0x00476553 | override::prt_476553_f3434af | USER_DEFINED |
| 0x00478adc | override::prt_478adc_180b3973 | USER_DEFINED |
| 0x0048305e | override::prt_48305e_f3434af | USER_DEFINED |
| 0x00483093 | override::prt_483093_f3434af | USER_DEFINED |
| 0x004832ba | override::prt_4832ba_f3434af | USER_DEFINED |
| 0x004832ef | override::prt_4832ef_f3434af | USER_DEFINED |
| 0x004834ca | override::prt_4834ca_f3434af | USER_DEFINED |
| 0x004834ff | override::prt_4834ff_f3434af | USER_DEFINED |
| 0x00483645 | override::prt_483645_f3434af | USER_DEFINED |
| 0x0048367a | override::prt_48367a_f3434af | USER_DEFINED |
| 0x00483724 | override::prt_483724_f3434af | USER_DEFINED |
| 0x00484daf | override::prt_484daf_180cef6c | USER_DEFINED |
| 0x00484dd1 | override::prt_484dd1_f3434af | USER_DEFINED |
| 0x00484e11 | override::prt_484e11_a39eff91 | USER_DEFINED |
| 0x00484e3d | override::prt_484e3d_b7eeacb5 | USER_DEFINED |
| 0x00484e5f | override::prt_484e5f_f3434af | USER_DEFINED |
| 0x00484ef3 | override::prt_484ef3_b7eeacb5 | USER_DEFINED |
| 0x00484f31 | override::prt_484f31_f3434af | USER_DEFINED |
| 0x00484ffe | override::prt_484ffe_ea24a0a | USER_DEFINED |
| 0x00485102 | override::prt_485102_80b2ba94 | USER_DEFINED |
| 0x004852b9 | override::prt_4852b9_b93ca9d | USER_DEFINED |
| 0x004853c1 | override::prt_4853c1_a8b049c | USER_DEFINED |
| 0x00485421 | override::prt_485421_b7eeacb5 | USER_DEFINED |
| 0x0048544f | override::prt_48544f_b7eeacb5 | USER_DEFINED |
| 0x004854ee | override::prt_4854ee_7a0add2 | USER_DEFINED |
| 0x00485a5a | override::prt_485a5a_b7eeacb5 | USER_DEFINED |
| 0x00485bea | override::prt_485bea_b7eeacb5 | USER_DEFINED |
| 0x00485c33 | override::prt_485c33_b7eeacb5 | USER_DEFINED |
| 0x0048cd79 | override::prt_48cd79_aee7b64a | USER_DEFINED |
| 0x0048e254 | override::prt_48e254_aee7b64a | USER_DEFINED |
| 0x0048e270 | override::prt_48e270_130e8acb | USER_DEFINED |
| 0x0048e760 | override::prt_48e760_558b9120 | USER_DEFINED |
| 0x0048e8fe | override::prt_48e8fe_558b9120 | USER_DEFINED |
| 0x0048ea8e | override::prt_48ea8e_558b9120 | USER_DEFINED |
| 0x0048ec1e | override::prt_48ec1e_558b9120 | USER_DEFINED |
| 0x0048edce | override::prt_48edce_558b9120 | USER_DEFINED |
| 0x0048ef60 | override::prt_48ef60_558b9120 | USER_DEFINED |
| 0x0048f0fe | override::prt_48f0fe_558b9120 | USER_DEFINED |
| 0x0048f2a7 | override::prt_48f2a7_2826581e | USER_DEFINED |
| 0x004db180 | override::prt_4db180_8d2134c3 | USER_DEFINED |
| 0x004db50a | override::prt_4db50a_f3434af | USER_DEFINED |
| 0x004e272f | override::prt_4e272f_f3434af | USER_DEFINED |
| 0x004e27e9 | override::prt_4e27e9_f3434af | USER_DEFINED |
| 0x004e29f5 | override::prt_4e29f5_f3434af | USER_DEFINED |
| 0x004e2af3 | override::prt_4e2af3_f3434af | USER_DEFINED |
| 0x004e2b9b | override::prt_4e2b9b_f3434af | USER_DEFINED |
| 0x004e3522 | override::prt_4e3522_f3434af | USER_DEFINED |
| 0x004e395d | override::prt_4e395d_f3434af | USER_DEFINED |
| 0x004e39d1 | override::prt_4e39d1_f3434af | USER_DEFINED |
| 0x0050bd12 | override::prt_50bd12_e40233a0 | USER_DEFINED |
| 0x0050bd2b | override::prt_50bd2b_8d2134c3 | USER_DEFINED |
| 0x0050bd44 | override::prt_50bd44_8d2134c3 | USER_DEFINED |
| 0x0050bd66 | override::prt_50bd66_e40233a0 | USER_DEFINED |
| 0x0050bd7f | override::prt_50bd7f_8d2134c3 | USER_DEFINED |
| 0x0050bd98 | override::prt_50bd98_8d2134c3 | USER_DEFINED |
| 0x005120e0 | override::prt_5120e0_aee7b64a | USER_DEFINED |
| 0x005329b3 | override::prt_5329b3_aee7b64a | USER_DEFINED |
| 0x00532d2b | override::prt_532d2b_aee7b64a | USER_DEFINED |
| 0x00555a2e | override::prt_555a2e_1ab07511 | USER_DEFINED |
| 0x0055652b | override::prt_55652b_f3434af | USER_DEFINED |
| 0x0055660b | override::prt_55660b_f3434af | USER_DEFINED |
| 0x00560421 | override::prt_560421_aee7b64a | USER_DEFINED |
| 0x00560670 | override::prt_560670_aee7b64a | USER_DEFINED |
| 0x005609b5 | override::prt_5609b5_aee7b64a | USER_DEFINED |
| 0x0064ecba | override::prt_64ecba_180cef6c | USER_DEFINED |
| 0x0064ed70 | override::prt_64ed70_ea24a0a | USER_DEFINED |
| 0x0064ee80 | override::prt_64ee80_b7eeacb5 | USER_DEFINED |
| 0x0064ef5c | override::prt_64ef5c_180cef6c | USER_DEFINED |
| 0x0064f064 | override::prt_64f064_b7eeacb5 | USER_DEFINED |
| 0x0064f1b3 | override::prt_64f1b3_b93ca9d | USER_DEFINED |
| 0x0064f67e | override::prt_64f67e_c5df58b6 | USER_DEFINED |
| 0x006ccc79 | override::prt_6ccc79_be320b62 | USER_DEFINED |
| 0x006d4ea4 | override::prt_6d4ea4_7da4b6b7 | USER_DEFINED |
| 0x006d4ec3 | override::prt_6d4ec3_407e704c | USER_DEFINED |
| 0x006d5044 | override::prt_6d5044_7da4b6b7 | USER_DEFINED |
| 0x006d5063 | override::prt_6d5063_407e704c | USER_DEFINED |
| 0x006f42e9 | override::prt_6f42e9_180cef6c | USER_DEFINED |
| 0x006f4457 | override::prt_6f4457_ea24a0a | USER_DEFINED |
| 0x006f45a3 | override::prt_6f45a3_558b9120 | USER_DEFINED |
| 0x006f4645 | override::prt_6f4645_180cef6c | USER_DEFINED |
| 0x006f46c7 | override::prt_6f46c7_558b9120 | USER_DEFINED |
| 0x006f4797 | override::prt_6f4797_b93ca9d | USER_DEFINED |
| 0x006f4f0f | override::prt_6f4f0f_180cef6c | USER_DEFINED |
| 0x006f50c9 | override::prt_6f50c9_ea24a0a | USER_DEFINED |
| 0x006f5215 | override::prt_6f5215_558b9120 | USER_DEFINED |
| 0x006f5286 | override::prt_6f5286_180cef6c | USER_DEFINED |
| 0x006f5305 | override::prt_6f5305_558b9120 | USER_DEFINED |
| 0x006f53d8 | override::prt_6f53d8_b93ca9d | USER_DEFINED |
| 0x0070c231 | override::prt_70c231_aee7b64a | USER_DEFINED |
| 0x0070c466 | override::prt_70c466_be320b62 | USER_DEFINED |
| 0x0070c530 | override::prt_70c530_aee7b64a | USER_DEFINED |
| 0x007162c2 | override::prt_7162c2_aee7b64a | USER_DEFINED |
| 0x0071679a | override::prt_71679a_aee7b64a | USER_DEFINED |
| 0x007169e9 | override::prt_7169e9_aee7b64a | USER_DEFINED |
| 0x0071b492 | override::prt_71b492_a39eff91 | USER_DEFINED |
| 0x0071b5da | override::prt_71b5da_a39eff91 | USER_DEFINED |
| 0x0071b716 | override::prt_71b716_a39eff91 | USER_DEFINED |
| 0x0071b856 | override::prt_71b856_a39eff91 | USER_DEFINED |
| 0x0071b956 | override::prt_71b956_b7eeacb5 | USER_DEFINED |
| 0x0071bb7e | override::prt_71bb7e_b7eeacb5 | USER_DEFINED |
| 0x0071bd9a | override::prt_71bd9a_b7eeacb5 | USER_DEFINED |
| 0x0071bfba | override::prt_71bfba_b7eeacb5 | USER_DEFINED |
| 0x0071f8aa | override::prt_71f8aa_b7eeacb5 | USER_DEFINED |
| 0x0071fb00 | override::prt_71fb00_ebf76999 | USER_DEFINED |
| 0x0071fbca | override::prt_71fbca_b7eeacb5 | USER_DEFINED |
| 0x0071fe03 | override::prt_71fe03_ebf76999 | USER_DEFINED |
| 0x0071fecd | override::prt_71fecd_b7eeacb5 | USER_DEFINED |
| 0x00720106 | override::prt_720106_ebf76999 | USER_DEFINED |
| 0x007201d0 | override::prt_7201d0_b7eeacb5 | USER_DEFINED |
| 0x0072062f | override::prt_72062f_d9a57125 | USER_DEFINED |
| 0x007264e4 | override::prt_7264e4_f3434af | USER_DEFINED |
| 0x00726b13 | override::prt_726b13_68003723 | USER_DEFINED |
| 0x007305ec | override::prt_7305ec_8cf8befb | USER_DEFINED |
| 0x00730fb0 | override::prt_730fb0_c32105f2 | USER_DEFINED |
| 0x00733720 | override::prt_733720_8d2134c3 | USER_DEFINED |
| 0x0073baee | override::prt_73baee_f3434af | USER_DEFINED |
| 0x0073bb0b | override::prt_73bb0b_f3434af | USER_DEFINED |
| 0x0073bb25 | override::prt_73bb25_f3434af | USER_DEFINED |
| 0x0073c14e | override::prt_73c14e_f3434af | USER_DEFINED |
| 0x0073c16b | override::prt_73c16b_f3434af | USER_DEFINED |
| 0x0073c185 | override::prt_73c185_f3434af | USER_DEFINED |
| 0x0073c1a2 | override::prt_73c1a2_f3434af | USER_DEFINED |
| 0x0073cf91 | override::prt_73cf91_7de13c66 | USER_DEFINED |
| 0x0073cfe2 | override::prt_73cfe2_7de13c66 | USER_DEFINED |
| 0x0073d6b2 | override::prt_73d6b2_f3434af | USER_DEFINED |
| 0x0073d6d7 | override::prt_73d6d7_407e704c | USER_DEFINED |
| 0x0073e54b | override::prt_73e54b_f3434af | USER_DEFINED |
| 0x0074603e | override::prt_74603e_4bfc86ce | USER_DEFINED |
| 0x0074605c | override::prt_74605c_aee7b64a | USER_DEFINED |
| 0x0076a48d | override::prt_76a48d_f3434af | USER_DEFINED |
| 0x00776c29 | override::prt_776c29_1f968f02 | USER_DEFINED |
| 0x00789594 | override::prt_789594_f3434af | USER_DEFINED |
| 0x00789926 | override::prt_789926_f3434af | USER_DEFINED |
| 0x007a0a08 | override::prt_7a0a08_f3434af | USER_DEFINED |
| 0x007a1519 | override::prt_7a1519_8d2134c3 | USER_DEFINED |
| 0x007a7318 | override::prt_7a7318_8d2134c3 | USER_DEFINED |
| 0x007a84f3 | override::prt_7a84f3_ef7ef8bd | USER_DEFINED |
| 0x007a8573 | override::prt_7a8573_ef7ef8bd | USER_DEFINED |
| 0x007a9169 | override::prt_7a9169_ef7ef8bd | USER_DEFINED |
| 0x007a91b0 | override::prt_7a91b0_ef7ef8bd | USER_DEFINED |
| 0x007a926e | override::prt_7a926e_ef7ef8bd | USER_DEFINED |
| 0x007ab2b7 | override::prt_7ab2b7_b7eeacb5 | USER_DEFINED |
| 0x007ab467 | override::prt_7ab467_b7eeacb5 | USER_DEFINED |
| 0x007ab6b3 | override::prt_7ab6b3_b7eeacb5 | USER_DEFINED |
| 0x007abaaa | override::prt_7abaaa_b7eeacb5 | USER_DEFINED |
| 0x007abc27 | override::prt_7abc27_b7eeacb5 | USER_DEFINED |
| 0x007abcd7 | override::prt_7abcd7_b7eeacb5 | USER_DEFINED |
| 0x007abd87 | override::prt_7abd87_b7eeacb5 | USER_DEFINED |
| 0x007abe37 | override::prt_7abe37_b7eeacb5 | USER_DEFINED |
| 0x007b03b6 | override::prt_7b03b6_b7eeacb5 | USER_DEFINED |
| 0x007b04a1 | override::prt_7b04a1_558b9120 | USER_DEFINED |
| 0x007b0641 | override::prt_7b0641_b7eeacb5 | USER_DEFINED |
| 0x007b0689 | override::prt_7b0689_72cbc2df | USER_DEFINED |
| 0x007b086b | override::prt_7b086b_c719fb9e | USER_DEFINED |
| 0x007b0bbf | override::prt_7b0bbf_180cef6c | USER_DEFINED |
| 0x007b0d87 | override::prt_7b0d87_311ea623 | USER_DEFINED |
| 0x007b1104 | override::prt_7b1104_33cfa187 | USER_DEFINED |
| 0x007b1132 | override::prt_7b1132_33cfa187 | USER_DEFINED |
| 0x007b1160 | override::prt_7b1160_33cfa187 | USER_DEFINED |
| 0x007f2318 | override::prt_7f2318_f3434af | USER_DEFINED |
| 0x008024de | override::prt_8024de_33cfa187 | USER_DEFINED |
| 0x00802512 | override::prt_802512_33cfa187 | USER_DEFINED |

</details>

### symbols vs branch DB (3227 trusted)

agree 0 / conflict 0 / server-only 3227 (skipped 0 default-named)

<details><summary>server-only (first 200 of 3227)</summary>

| addr | server | source |
|---|---|---|
| 0x004010bd | override::prt_4010bd_8d2134c3 | USER_DEFINED |
| 0x00407461 | override::prt_407461_f3434af | USER_DEFINED |
| 0x004077b3 | override::prt_4077b3_f3434af | USER_DEFINED |
| 0x004077e3 | override::prt_4077e3_f3434af | USER_DEFINED |
| 0x004078c3 | override::prt_4078c3_ea24a0a | USER_DEFINED |
| 0x00408395 | override::prt_408395_33cfa187 | USER_DEFINED |
| 0x004253c8 | override::prt_4253c8_407e704c | USER_DEFINED |
| 0x0042546b | override::prt_42546b_407e704c | USER_DEFINED |
| 0x00428a24 | override::prt_428a24_8d2134c3 | USER_DEFINED |
| 0x00428dba | override::prt_428dba_8d2134c3 | USER_DEFINED |
| 0x0042902e | override::prt_42902e_558b9120 | USER_DEFINED |
| 0x00458fc5 | override::prt_458fc5_f3434af | USER_DEFINED |
| 0x0045900e | override::prt_45900e_407e704c | USER_DEFINED |
| 0x00460c67 | override::prt_460c67_7de13c66 | USER_DEFINED |
| 0x00460caf | override::prt_460caf_9907cd9f | USER_DEFINED |
| 0x00460e97 | override::prt_460e97_7de13c66 | USER_DEFINED |
| 0x00460edf | override::prt_460edf_9907cd9f | USER_DEFINED |
| 0x0046393b | override::prt_46393b_c448bc6a | USER_DEFINED |
| 0x004646f9 | override::prt_4646f9_f3434af | USER_DEFINED |
| 0x00464742 | override::prt_464742_407e704c | USER_DEFINED |
| 0x00464772 | override::prt_464772_8d2134c3 | USER_DEFINED |
| 0x0046f680 | override::prt_46f680_f3434af | USER_DEFINED |
| 0x0046fc47 | override::prt_46fc47_dbe336c3 | USER_DEFINED |
| 0x0046fcb0 | override::prt_46fcb0_f3434af | USER_DEFINED |
| 0x0046fd6b | override::prt_46fd6b_aee7b64a | USER_DEFINED |
| 0x0046fe01 | override::prt_46fe01_50dddcd6 | USER_DEFINED |
| 0x0046fe7a | override::prt_46fe7a_7de13c66 | USER_DEFINED |
| 0x004700b0 | override::prt_4700b0_ebf76999 | USER_DEFINED |
| 0x00470173 | override::prt_470173_ebf76999 | USER_DEFINED |
| 0x0047050f | override::prt_47050f_f3434af | USER_DEFINED |
| 0x00470df8 | override::prt_470df8_e40233a0 | USER_DEFINED |
| 0x00474a8a | override::prt_474a8a_aee7b64a | USER_DEFINED |
| 0x00474b94 | override::prt_474b94_8d2134c3 | USER_DEFINED |
| 0x00474ee6 | override::prt_474ee6_b7eeacb5 | USER_DEFINED |
| 0x004751f4 | override::prt_4751f4_f3434af | USER_DEFINED |
| 0x00475632 | override::prt_475632_f3434af | USER_DEFINED |
| 0x00475b7b | override::prt_475b7b_4bfc86ce | USER_DEFINED |
| 0x00476468 | override::prt_476468_f3434af | USER_DEFINED |
| 0x00476553 | override::prt_476553_f3434af | USER_DEFINED |
| 0x00478adc | override::prt_478adc_180b3973 | USER_DEFINED |
| 0x0048305e | override::prt_48305e_f3434af | USER_DEFINED |
| 0x00483093 | override::prt_483093_f3434af | USER_DEFINED |
| 0x004832ba | override::prt_4832ba_f3434af | USER_DEFINED |
| 0x004832ef | override::prt_4832ef_f3434af | USER_DEFINED |
| 0x004834ca | override::prt_4834ca_f3434af | USER_DEFINED |
| 0x004834ff | override::prt_4834ff_f3434af | USER_DEFINED |
| 0x00483645 | override::prt_483645_f3434af | USER_DEFINED |
| 0x0048367a | override::prt_48367a_f3434af | USER_DEFINED |
| 0x00483724 | override::prt_483724_f3434af | USER_DEFINED |
| 0x00484daf | override::prt_484daf_180cef6c | USER_DEFINED |
| 0x00484dd1 | override::prt_484dd1_f3434af | USER_DEFINED |
| 0x00484e11 | override::prt_484e11_a39eff91 | USER_DEFINED |
| 0x00484e3d | override::prt_484e3d_b7eeacb5 | USER_DEFINED |
| 0x00484e5f | override::prt_484e5f_f3434af | USER_DEFINED |
| 0x00484ef3 | override::prt_484ef3_b7eeacb5 | USER_DEFINED |
| 0x00484f31 | override::prt_484f31_f3434af | USER_DEFINED |
| 0x00484ffe | override::prt_484ffe_ea24a0a | USER_DEFINED |
| 0x00485102 | override::prt_485102_80b2ba94 | USER_DEFINED |
| 0x004852b9 | override::prt_4852b9_b93ca9d | USER_DEFINED |
| 0x004853c1 | override::prt_4853c1_a8b049c | USER_DEFINED |
| 0x00485421 | override::prt_485421_b7eeacb5 | USER_DEFINED |
| 0x0048544f | override::prt_48544f_b7eeacb5 | USER_DEFINED |
| 0x004854ee | override::prt_4854ee_7a0add2 | USER_DEFINED |
| 0x00485a5a | override::prt_485a5a_b7eeacb5 | USER_DEFINED |
| 0x00485bea | override::prt_485bea_b7eeacb5 | USER_DEFINED |
| 0x00485c33 | override::prt_485c33_b7eeacb5 | USER_DEFINED |
| 0x0048cd79 | override::prt_48cd79_aee7b64a | USER_DEFINED |
| 0x0048e254 | override::prt_48e254_aee7b64a | USER_DEFINED |
| 0x0048e270 | override::prt_48e270_130e8acb | USER_DEFINED |
| 0x0048e760 | override::prt_48e760_558b9120 | USER_DEFINED |
| 0x0048e8fe | override::prt_48e8fe_558b9120 | USER_DEFINED |
| 0x0048ea8e | override::prt_48ea8e_558b9120 | USER_DEFINED |
| 0x0048ec1e | override::prt_48ec1e_558b9120 | USER_DEFINED |
| 0x0048edce | override::prt_48edce_558b9120 | USER_DEFINED |
| 0x0048ef60 | override::prt_48ef60_558b9120 | USER_DEFINED |
| 0x0048f0fe | override::prt_48f0fe_558b9120 | USER_DEFINED |
| 0x0048f2a7 | override::prt_48f2a7_2826581e | USER_DEFINED |
| 0x004db180 | override::prt_4db180_8d2134c3 | USER_DEFINED |
| 0x004db50a | override::prt_4db50a_f3434af | USER_DEFINED |
| 0x004e272f | override::prt_4e272f_f3434af | USER_DEFINED |
| 0x004e27e9 | override::prt_4e27e9_f3434af | USER_DEFINED |
| 0x004e29f5 | override::prt_4e29f5_f3434af | USER_DEFINED |
| 0x004e2af3 | override::prt_4e2af3_f3434af | USER_DEFINED |
| 0x004e2b9b | override::prt_4e2b9b_f3434af | USER_DEFINED |
| 0x004e3522 | override::prt_4e3522_f3434af | USER_DEFINED |
| 0x004e395d | override::prt_4e395d_f3434af | USER_DEFINED |
| 0x004e39d1 | override::prt_4e39d1_f3434af | USER_DEFINED |
| 0x0050bd12 | override::prt_50bd12_e40233a0 | USER_DEFINED |
| 0x0050bd2b | override::prt_50bd2b_8d2134c3 | USER_DEFINED |
| 0x0050bd44 | override::prt_50bd44_8d2134c3 | USER_DEFINED |
| 0x0050bd66 | override::prt_50bd66_e40233a0 | USER_DEFINED |
| 0x0050bd7f | override::prt_50bd7f_8d2134c3 | USER_DEFINED |
| 0x0050bd98 | override::prt_50bd98_8d2134c3 | USER_DEFINED |
| 0x005120e0 | override::prt_5120e0_aee7b64a | USER_DEFINED |
| 0x005329b3 | override::prt_5329b3_aee7b64a | USER_DEFINED |
| 0x00532d2b | override::prt_532d2b_aee7b64a | USER_DEFINED |
| 0x00555a2e | override::prt_555a2e_1ab07511 | USER_DEFINED |
| 0x0055652b | override::prt_55652b_f3434af | USER_DEFINED |
| 0x0055660b | override::prt_55660b_f3434af | USER_DEFINED |
| 0x00560421 | override::prt_560421_aee7b64a | USER_DEFINED |
| 0x00560670 | override::prt_560670_aee7b64a | USER_DEFINED |
| 0x005609b5 | override::prt_5609b5_aee7b64a | USER_DEFINED |
| 0x0064ecba | override::prt_64ecba_180cef6c | USER_DEFINED |
| 0x0064ed70 | override::prt_64ed70_ea24a0a | USER_DEFINED |
| 0x0064ee80 | override::prt_64ee80_b7eeacb5 | USER_DEFINED |
| 0x0064ef5c | override::prt_64ef5c_180cef6c | USER_DEFINED |
| 0x0064f064 | override::prt_64f064_b7eeacb5 | USER_DEFINED |
| 0x0064f1b3 | override::prt_64f1b3_b93ca9d | USER_DEFINED |
| 0x0064f67e | override::prt_64f67e_c5df58b6 | USER_DEFINED |
| 0x006ccc79 | override::prt_6ccc79_be320b62 | USER_DEFINED |
| 0x006d4ea4 | override::prt_6d4ea4_7da4b6b7 | USER_DEFINED |
| 0x006d4ec3 | override::prt_6d4ec3_407e704c | USER_DEFINED |
| 0x006d5044 | override::prt_6d5044_7da4b6b7 | USER_DEFINED |
| 0x006d5063 | override::prt_6d5063_407e704c | USER_DEFINED |
| 0x006f42e9 | override::prt_6f42e9_180cef6c | USER_DEFINED |
| 0x006f4457 | override::prt_6f4457_ea24a0a | USER_DEFINED |
| 0x006f45a3 | override::prt_6f45a3_558b9120 | USER_DEFINED |
| 0x006f4645 | override::prt_6f4645_180cef6c | USER_DEFINED |
| 0x006f46c7 | override::prt_6f46c7_558b9120 | USER_DEFINED |
| 0x006f4797 | override::prt_6f4797_b93ca9d | USER_DEFINED |
| 0x006f4f0f | override::prt_6f4f0f_180cef6c | USER_DEFINED |
| 0x006f50c9 | override::prt_6f50c9_ea24a0a | USER_DEFINED |
| 0x006f5215 | override::prt_6f5215_558b9120 | USER_DEFINED |
| 0x006f5286 | override::prt_6f5286_180cef6c | USER_DEFINED |
| 0x006f5305 | override::prt_6f5305_558b9120 | USER_DEFINED |
| 0x006f53d8 | override::prt_6f53d8_b93ca9d | USER_DEFINED |
| 0x0070c231 | override::prt_70c231_aee7b64a | USER_DEFINED |
| 0x0070c466 | override::prt_70c466_be320b62 | USER_DEFINED |
| 0x0070c530 | override::prt_70c530_aee7b64a | USER_DEFINED |
| 0x007162c2 | override::prt_7162c2_aee7b64a | USER_DEFINED |
| 0x0071679a | override::prt_71679a_aee7b64a | USER_DEFINED |
| 0x007169e9 | override::prt_7169e9_aee7b64a | USER_DEFINED |
| 0x0071b492 | override::prt_71b492_a39eff91 | USER_DEFINED |
| 0x0071b5da | override::prt_71b5da_a39eff91 | USER_DEFINED |
| 0x0071b716 | override::prt_71b716_a39eff91 | USER_DEFINED |
| 0x0071b856 | override::prt_71b856_a39eff91 | USER_DEFINED |
| 0x0071b956 | override::prt_71b956_b7eeacb5 | USER_DEFINED |
| 0x0071bb7e | override::prt_71bb7e_b7eeacb5 | USER_DEFINED |
| 0x0071bd9a | override::prt_71bd9a_b7eeacb5 | USER_DEFINED |
| 0x0071bfba | override::prt_71bfba_b7eeacb5 | USER_DEFINED |
| 0x0071f8aa | override::prt_71f8aa_b7eeacb5 | USER_DEFINED |
| 0x0071fb00 | override::prt_71fb00_ebf76999 | USER_DEFINED |
| 0x0071fbca | override::prt_71fbca_b7eeacb5 | USER_DEFINED |
| 0x0071fe03 | override::prt_71fe03_ebf76999 | USER_DEFINED |
| 0x0071fecd | override::prt_71fecd_b7eeacb5 | USER_DEFINED |
| 0x00720106 | override::prt_720106_ebf76999 | USER_DEFINED |
| 0x007201d0 | override::prt_7201d0_b7eeacb5 | USER_DEFINED |
| 0x0072062f | override::prt_72062f_d9a57125 | USER_DEFINED |
| 0x007264e4 | override::prt_7264e4_f3434af | USER_DEFINED |
| 0x00726b13 | override::prt_726b13_68003723 | USER_DEFINED |
| 0x007305ec | override::prt_7305ec_8cf8befb | USER_DEFINED |
| 0x00730fb0 | override::prt_730fb0_c32105f2 | USER_DEFINED |
| 0x00733720 | override::prt_733720_8d2134c3 | USER_DEFINED |
| 0x0073baee | override::prt_73baee_f3434af | USER_DEFINED |
| 0x0073bb0b | override::prt_73bb0b_f3434af | USER_DEFINED |
| 0x0073bb25 | override::prt_73bb25_f3434af | USER_DEFINED |
| 0x0073c14e | override::prt_73c14e_f3434af | USER_DEFINED |
| 0x0073c16b | override::prt_73c16b_f3434af | USER_DEFINED |
| 0x0073c185 | override::prt_73c185_f3434af | USER_DEFINED |
| 0x0073c1a2 | override::prt_73c1a2_f3434af | USER_DEFINED |
| 0x0073cf91 | override::prt_73cf91_7de13c66 | USER_DEFINED |
| 0x0073cfe2 | override::prt_73cfe2_7de13c66 | USER_DEFINED |
| 0x0073d6b2 | override::prt_73d6b2_f3434af | USER_DEFINED |
| 0x0073d6d7 | override::prt_73d6d7_407e704c | USER_DEFINED |
| 0x0073e54b | override::prt_73e54b_f3434af | USER_DEFINED |
| 0x0074603e | override::prt_74603e_4bfc86ce | USER_DEFINED |
| 0x0074605c | override::prt_74605c_aee7b64a | USER_DEFINED |
| 0x0076a48d | override::prt_76a48d_f3434af | USER_DEFINED |
| 0x00776c29 | override::prt_776c29_1f968f02 | USER_DEFINED |
| 0x00789594 | override::prt_789594_f3434af | USER_DEFINED |
| 0x00789926 | override::prt_789926_f3434af | USER_DEFINED |
| 0x007a0a08 | override::prt_7a0a08_f3434af | USER_DEFINED |
| 0x007a1519 | override::prt_7a1519_8d2134c3 | USER_DEFINED |
| 0x007a7318 | override::prt_7a7318_8d2134c3 | USER_DEFINED |
| 0x007a84f3 | override::prt_7a84f3_ef7ef8bd | USER_DEFINED |
| 0x007a8573 | override::prt_7a8573_ef7ef8bd | USER_DEFINED |
| 0x007a9169 | override::prt_7a9169_ef7ef8bd | USER_DEFINED |
| 0x007a91b0 | override::prt_7a91b0_ef7ef8bd | USER_DEFINED |
| 0x007a926e | override::prt_7a926e_ef7ef8bd | USER_DEFINED |
| 0x007ab2b7 | override::prt_7ab2b7_b7eeacb5 | USER_DEFINED |
| 0x007ab467 | override::prt_7ab467_b7eeacb5 | USER_DEFINED |
| 0x007ab6b3 | override::prt_7ab6b3_b7eeacb5 | USER_DEFINED |
| 0x007abaaa | override::prt_7abaaa_b7eeacb5 | USER_DEFINED |
| 0x007abc27 | override::prt_7abc27_b7eeacb5 | USER_DEFINED |
| 0x007abcd7 | override::prt_7abcd7_b7eeacb5 | USER_DEFINED |
| 0x007abd87 | override::prt_7abd87_b7eeacb5 | USER_DEFINED |
| 0x007abe37 | override::prt_7abe37_b7eeacb5 | USER_DEFINED |
| 0x007b03b6 | override::prt_7b03b6_b7eeacb5 | USER_DEFINED |
| 0x007b04a1 | override::prt_7b04a1_558b9120 | USER_DEFINED |
| 0x007b0641 | override::prt_7b0641_b7eeacb5 | USER_DEFINED |
| 0x007b0689 | override::prt_7b0689_72cbc2df | USER_DEFINED |
| 0x007b086b | override::prt_7b086b_c719fb9e | USER_DEFINED |
| 0x007b0bbf | override::prt_7b0bbf_180cef6c | USER_DEFINED |
| 0x007b0d87 | override::prt_7b0d87_311ea623 | USER_DEFINED |
| 0x007b1104 | override::prt_7b1104_33cfa187 | USER_DEFINED |
| 0x007b1132 | override::prt_7b1132_33cfa187 | USER_DEFINED |
| 0x007b1160 | override::prt_7b1160_33cfa187 | USER_DEFINED |
| 0x007f2318 | override::prt_7f2318_f3434af | USER_DEFINED |
| 0x008024de | override::prt_8024de_33cfa187 | USER_DEFINED |
| 0x00802512 | override::prt_802512_33cfa187 | USER_DEFINED |

</details>

## (b) GOG: trusted server names vs GOG address DB

agree 8385 / disagree 1940 / server-only 98 / db-only 80 (of which 0 are server functions with an untrusted name, 80 have no server function)

### GOG label suspects (disagree list)

| addr | server name | db name(s) | server source |
|---|---|---|---|
| 0x004072b0 | IsGameModded | Global::IsGameModded | USER_DEFINED |
| 0x00407340 | GetOverridePath | Global::GetOverridePath | USER_DEFINED |
| 0x004073c0 | SetClearColor | Global::SetClearColor | USER_DEFINED |
| 0x00407440 | ClearLogCounters | Global::ClearLogCounters | USER_DEFINED |
| 0x00407450 | WinMain | Global::WinMain | USER_DEFINED |
| 0x00408250 | glClear_glClear_int_engineMask | Global::glClear_glClear_int_engineMask | USER_DEFINED |
| 0x004082b0 | InitGameApp | Global::InitGameApp | USER_DEFINED |
| 0x00408600 | InitOpenGLWindow | Global::InitOpenGLWindow | USER_DEFINED |
| 0x00408740 | ProcessCommandLine | Global::ProcessCommandLine | USER_DEFINED |
| 0x00408910 | SetWindowTitle | Global::SetWindowTitle | USER_DEFINED |
| 0x004089f0 | DoConsole | Global::DoConsole | USER_DEFINED |
| 0x00409000 | messagepump | Global::messagepump | USER_DEFINED |
| 0x00409060 | SpawnBlackScreen | Global::SpawnBlackScreen | USER_DEFINED |
| 0x00409260 | ReInitAurora | Global::ReInitAurora | USER_DEFINED |
| 0x00409920 | DoConsoleHist | Global::DoConsoleHist | USER_DEFINED |
| 0x004099e0 | UpdateScreen | Global::UpdateScreen | USER_DEFINED |
| 0x00409c70 | HandleWMKeyDownMessage | Global::HandleWMKeyDownMessage | USER_DEFINED |
| 0x00409f70 | HandleWMCharMessage | Global::HandleWMCharMessage | USER_DEFINED |
| 0x0040a260 | WinMessageHandler | Global::WinMessageHandler | USER_DEFINED |
| 0x0040b740 | SetupOpenGL | Global::SetupOpenGL | USER_DEFINED |
| 0x0040b750 | DeactivateRenderWindow | Global::DeactivateRenderWindow | USER_DEFINED |
| 0x0040b800 | ActivateRenderWindow | Global::ActivateRenderWindow | USER_DEFINED |
| 0x0040b960 | ChangeResolutionForMovie | Global::ChangeResolutionForMovie | USER_DEFINED |
| 0x0040ba50 | RestoreResolutionFromMovie | Global::RestoreResolutionFromMovie | USER_DEFINED |
| 0x0040bba0 | StoreMouseTrailsState | Global::StoreMouseTrailsState | USER_DEFINED |
| 0x0040bc10 | StoreScreenSaverState | Global::StoreScreenSaverState | USER_DEFINED |
| 0x0040c380 | IsJoystickConnected | Global::IsJoystickConnected | USER_DEFINED |
| 0x0040e5a0 | OnGameComplete | Global::OnGameComplete | USER_DEFINED |
| 0x0040e920 | CExoArrayList<CExoString>::Add | CExoArrayList_CExoString_::Add | USER_DEFINED |
| 0x0040eba0 | FootstepEvent | Global::FootstepEvent | USER_DEFINED |
| 0x0040ebc0 | Swing | Global::Swing | USER_DEFINED |
| 0x0040ec10 | SwingShortEvent | Global::SwingShortEvent | USER_DEFINED |
| 0x0040ec80 | SwingLongEvent | Global::SwingLongEvent | USER_DEFINED |
| 0x0040ecf0 | SwingTwirlEvent | Global::SwingTwirlEvent | USER_DEFINED |
| 0x0040ed60 | HitClashEvent | Global::HitClashEvent | USER_DEFINED |
| 0x0040edd0 | HitContactEvent | Global::HitContactEvent | USER_DEFINED |
| 0x0040ee20 | HitParryEvent | Global::HitParryEvent | USER_DEFINED |
| 0x0040ee90 | HitEvent | Global::HitEvent | USER_DEFINED |
| 0x0040f4a0 | SwitchWeaponEvent | Global::SwitchWeaponEvent | USER_DEFINED |
| 0x0040fcc0 | HitGroundEvent | Global::HitGroundEvent | USER_DEFINED |
| 0x00410c20 | return_zero | Global::return_zero | USER_DEFINED |
| 0x00410d30 | CSWCCreature::GetPortrait | CSWCCreature::GetPortrait_2 | USER_DEFINED |
| 0x00412180 | CSWCCreature::AIUpdate | CSWCCreature::AIUpdate_2 | USER_DEFINED |
| 0x00414410 | Unblur | Global::Unblur | USER_DEFINED |
| 0x00419dc0 | CSWCCreature::UpdateRelativePerception | CSWCCreature::UpdateRelativePerception_2 | USER_DEFINED |
| 0x0041b7b0 | CSWCCreature::UpdateRangedAttackData | CSWCCreature::UpdateRangedAttackData_2 | USER_DEFINED |
| 0x0041f840 | CSWCCreature::SetSpecialWalkAnimation | CSWCCreature::SetSpecialWalkAnimation_2 | USER_DEFINED |
| 0x0041fa70 | GetCreatureRadius | Global::GetCreatureRadius | USER_DEFINED |
| 0x00422ea0 | SetQuededAnimationCallback | Global::SetQuededAnimationCallback | USER_DEFINED |
| 0x00422f20 | ClearQuededAnimationCallback | Global::ClearQuededAnimationCallback | USER_DEFINED |
| 0x00424370 | SceneHitCheck | Global::SceneHitCheck | USER_DEFINED |
| 0x00424b60 | CSWCCreature::GetEquipmentForcePowers | CSWCCreature::GetEquipmentForcePowers_2 | USER_DEFINED |
| 0x004254a0 | CSWGuiInterfaceAction_UseForcePower | Global::CSWGuiInterfaceAction_UseForcePower | USER_DEFINED |
| 0x004282c0 | CSWCCreature::CreateHostileItemEntry | CSWCCreature::CreateHostileItemEntry_2 | USER_DEFINED |
| 0x00428690 | CSWCCreature::GetHostileItems | CSWCCreature::GetHostileItems_2 | USER_DEFINED |
| 0x0042bb30 | CSWCCreature::UpdateRollingFootstepSound | CSWCCreature::UpdateRollingFootstepSound_2 | USER_DEFINED |
| 0x0042c190 | CExoArrayList<CSWGuiInterfaceAction>::SetSize | CExoArrayList_CSWGuiInterfaceAction_::SetSize | USER_DEFINED |
| 0x0042c6f0 | IsPsButtonIcons | Global::IsPsButtonIcons | USER_DEFINED |
| 0x0042c720 | SetFramesPerSecond | Global::SetFramesPerSecond | USER_DEFINED |
| 0x00431750 | CClientExoAppInternal::DestroyQuestJournal | CClientExoAppInternal::DestroyQuestJournal_2 | USER_DEFINED |
| 0x004317f0 | CClientExoAppInternal::AddToPauseExclusionList | CClientExoAppInternal::AddToPauseExclusionList_2 | USER_DEFINED |
| 0x0043a920 | CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList | CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2 | USER_DEFINED |
| 0x00449d70 | CClientExoAppInternal::UpdateSoundEngine | CClientExoAppInternal::UpdateSoundEngine_2 | USER_DEFINED |
| 0x0044ce60 | FindMovieInOverrides | Global::FindMovieInOverrides | USER_DEFINED |
| 0x0044dce0 | CExoArrayList<CExoString>::CExoArrayList | CExoArrayList_CExoString_::CExoArrayList | USER_DEFINED |
| 0x0044dd60 | CExoArrayList<CExoString>::CExoArrayList | CExoArrayList_CExoString_::CExoArrayList_2 | USER_DEFINED |
| 0x0044dda0 | CExoArrayList<CExoString>::~CExoArrayList | CExoArrayList_CExoString_::Destructor | USER_DEFINED |
| 0x0044dde0 | CExoArrayList<CExoString>::AddUnique | CExoArrayList_CExoString_::AddUnique | USER_DEFINED |
| 0x0044de70 | CClientExoAppInternal::IsMoviePlaying | CClientExoAppInternal::IsMoviePlaying_2 | USER_DEFINED |
| 0x0044ded0 | CClientExoAppInternal::InitializeMoviePlayer | CClientExoAppInternal::InitializeMoviePlayer_2 | USER_DEFINED |
| 0x0044e030 | CExoArrayList<CSWGuiSkillStruct>::CExoArrayList | CExoArrayList_CSWGuiSkillStruct_::CExoArrayList | USER_DEFINED |
| 0x0044e070 | CClientExoAppInternal::AddMoveToModuleMovie | CClientExoAppInternal::AddMoveToModuleMovie_2 | USER_DEFINED |
| 0x0044e110 | CExoArrayList<CGuiStringStruct>::Insert | CExoArrayList_CGuiStringStruct_::Insert | USER_DEFINED |
| 0x0044e4b0 | CSWRumblePattern::~CSWRumblePattern | CSWRumblePattern::Destructor_2 | USER_DEFINED |
| 0x0044e510 | CExoArrayList<CExoString>::Contains | CExoArrayList_CExoString_::Contains | USER_DEFINED |
| 0x0044e7b0 | CExoArrayList<CGuiStringStruct>::Allocate | CExoArrayList_CGuiStringStruct_::Allocate | USER_DEFINED |
| 0x0044ef10 | PlayRoomSoundEvent | Global::PlayRoomSoundEvent | USER_DEFINED |
| 0x0044f630 | CGameObject::~CGameObject | CGameObject::Destructor_2 | USER_DEFINED |
| 0x0044f6b0 | CSWCArea::~CSWCArea | CSWCArea::Destructor_2 | USER_DEFINED |
| 0x00457590 | CSWCArea::GetRoom | CSWCArea::GetRoom_2 | USER_DEFINED |
| 0x004575d0 | CSWCArea::GetRoom | CSWCArea::GetRoom_3 | USER_DEFINED |
| 0x00457ac0 | CExoArrayList<Vector>::CExoArrayList | CExoArrayList_Vector_::CExoArrayList | USER_DEFINED |
| 0x00457b00 | CExoArrayList<Vector>::Allocate | CExoArrayList_Vector_::Allocate | USER_DEFINED |
| 0x00457b60 | CExoArrayList<CSWCMapPin*>::Allocate | CExoArrayList_CSWCMapPin__::Allocate | USER_DEFINED |
| 0x00457ce0 | GetCheatsTurnedOn | Global::GetCheatsTurnedOn | USER_DEFINED |
| 0x00457e60 | heal | Global::heal | USER_DEFINED |
| 0x00457ea0 | CheatSetSkill | Global::CheatSetSkill | USER_DEFINED |
| 0x00457f50 | setcomputeruse | Global::setcomputeruse | USER_DEFINED |
| 0x00457f70 | setdemolitions | Global::setdemolitions | USER_DEFINED |
| 0x00457f90 | setstealth | Global::setstealth | USER_DEFINED |
| 0x00457fb0 | setawareness | Global::setawareness | USER_DEFINED |
| 0x00457fd0 | setpersuade | Global::setpersuade | USER_DEFINED |
| 0x00457ff0 | setrepair | Global::setrepair | USER_DEFINED |
| 0x00458010 | setsecurity | Global::setsecurity | USER_DEFINED |
| 0x00458030 | settreatinjury | Global::settreatinjury | USER_DEFINED |
| 0x00458050 | CheatSetAbility | Global::CheatSetAbility | USER_DEFINED |
| 0x004581e0 | setstrength | Global::setstrength | USER_DEFINED |
| 0x00458200 | setdexterity | Global::setdexterity | USER_DEFINED |
| 0x00458220 | setconstitution | Global::setconstitution | USER_DEFINED |
| 0x00458240 | setintelligence | Global::setintelligence | USER_DEFINED |
| 0x00458260 | setwisdom | Global::setwisdom | USER_DEFINED |
| 0x00458280 | setcharisma | Global::setcharisma | USER_DEFINED |
| 0x004582a0 | addexp | Global::addexp | USER_DEFINED |
| 0x00458300 | turbo | Global::turbo | USER_DEFINED |
| 0x00458360 | invulnerability | Global::invulnerability | USER_DEFINED |
| 0x004583a0 | restartminigame | Global::restartminigame | USER_DEFINED |
| 0x00458410 | GetMiniGamePlayingType | Global::GetMiniGamePlayingType | USER_DEFINED |
| 0x00458480 | bright | Global::bright | USER_DEFINED |
| 0x004584f0 | addlightside | Global::addlightside | USER_DEFINED |
| 0x004585b0 | adddarkside | Global::adddarkside | USER_DEFINED |
| 0x00458680 | revealmap | Global::revealmap | USER_DEFINED |
| 0x004586a0 | givecredits | Global::givecredits | USER_DEFINED |
| 0x00458730 | giveitem | Global::giveitem | USER_DEFINED |
| 0x00458810 | givemed | Global::givemed | USER_DEFINED |
| 0x004588c0 | giverepair | Global::giverepair | USER_DEFINED |
| 0x00458970 | givecomspikes | Global::givecomspikes | USER_DEFINED |
| 0x00458a20 | givesecspikes | Global::givesecspikes | USER_DEFINED |
| 0x00458ad0 | givesitharmour | Global::givesitharmour | USER_DEFINED |
| 0x00458b20 | warp | Global::warp | USER_DEFINED |
| 0x00458c40 | infiniteuses | Global::infiniteuses | USER_DEFINED |
| 0x00458ca0 | whereami | Global::whereami | USER_DEFINED |
| 0x00458df0 | addlevel | Global::addlevel | USER_DEFINED |
| 0x00458f70 | dancedancemalak | Global::dancedancemalak | USER_DEFINED |
| 0x00458fb0 | lockcamera | Global::lockcamera | USER_DEFINED |
| 0x00462840 | CExoArrayList<CExoInputEvent>::Add | CExoArrayList_CExoInputEvent_::Add | USER_DEFINED |
| 0x004628c0 | CExoArrayList<CExoInputEvent>::Insert | CExoArrayList_CExoInputEvent_::Insert | USER_DEFINED |
| 0x00462970 | CExoArrayList<CExoInputEvent>::DelIndex | CExoArrayList_CExoInputEvent_::DelIndex | USER_DEFINED |
| 0x004629e0 | CExoArrayList<CExoInputEvent>::Allocate | CExoArrayList_CExoInputEvent_::Allocate | USER_DEFINED |
| 0x00468cc0 | GetMouseSenSetting | Global::GetMouseSenSetting | USER_DEFINED |
| 0x00469b40 | CGameObjectArray::Delete | CGameObjectArray::Delete_2 | USER_DEFINED |
| 0x0046a0b0 | CSWCModule_local_camera_target_Part_cleanup_helper | Global::CSWCModule_local_camera_target_Part_cleanup_helper | USER_DEFINED |
| 0x0046a0d0 | CSWCModule::~CSWCModule | CSWCModule::Destructor_2 | USER_DEFINED |
| 0x004707b0 | not_CSWCModule_IsCameraAnimated__animation_part_helper_004707B0 | Global::not_CSWCModule_IsCameraAnimated__animation_part_helper_004707B0 | USER_DEFINED |
| 0x004720d0 | SafePointerList<CAurObject>::AddPointer | SafePointerList_CAurObject_::AddPointer | USER_DEFINED |
| 0x00472750 | CSWCObject::~CSWCObject | CSWCObject::Destructor_2 | USER_DEFINED |
| 0x004727b0 | CSWCObject::~CSWCObject | CSWCObject::Destructor_3 | USER_DEFINED |
| 0x00472a30 | CSWCObject::ReplaceTexture | CSWCObject::ReplaceTexture_2 | USER_DEFINED |
| 0x004756d0 | CSWCObject::SetEnvMap | CSWCObject::SetEnvMap_2 | USER_DEFINED |
| 0x004758b0 | CSWCObject::RestoreTexture | CSWCObject::RestoreTexture_2 | USER_DEFINED |
| 0x004773c0 | QueuedCallbackFunction | Global::QueuedCallbackFunction | USER_DEFINED |
| 0x00477bf0 | CExoLinkedList<CSWCVisualEffectOnObject>::GetAtHead | CExoLinkedList_CSWCVisualEffectOnObject_::GetAtHead | USER_DEFINED |
| 0x00478eee | CSWParty::SetLeader | CSWParty::SetLeader_2 | USER_DEFINED |
| 0x00479870 | CSWParty::RemoveCharacter | CSWParty::RemoveCharacter_2 | USER_DEFINED |
| 0x0047e460 | CSWCItem::~CSWCItem | CSWCItem::Destructor_2 | USER_DEFINED |
| 0x0047eb80 | CSWCItem::UnloadModel | CSWCItem::UnloadModel_2 | USER_DEFINED |
| 0x0048a160 | CSWCCreatureAppearance | Global::CSWCCreatureAppearance | USER_DEFINED |
| 0x0048a750 | CreateHeadAppearance | Global::CreateHeadAppearance | USER_DEFINED |
| 0x0048a850 | CreateTailAppearance | Global::CreateTailAppearance | USER_DEFINED |
| 0x0048a960 | CreateWingAppearance | Global::CreateWingAppearance | USER_DEFINED |
| 0x0048b9d0 | CreateSimple | Global::CreateSimple | USER_DEFINED |
| 0x0048c120 | GetGenderChar | Global::GetGenderChar | USER_DEFINED |
| 0x0048c1e0 | GetAnimType | Global::GetAnimType | USER_DEFINED |
| 0x0048c2b0 | RequestBodyAppearance | Global::RequestBodyAppearance | USER_DEFINED |
| 0x0048c3c0 | RequestSimple | Global::RequestSimple | USER_DEFINED |
| 0x0048c520 | RequestBType | Global::RequestBType | USER_DEFINED |
| 0x0048c590 | RequestHeadAppearance | Global::RequestHeadAppearance | USER_DEFINED |
| 0x0048e340 | GetRunLooksAwfulRate | Global::GetRunLooksAwfulRate | USER_DEFINED |
| 0x0048ee50 | CSWCCreatureStats::~CSWCCreatureStats | CSWCCreatureStats::Destructor_2 | USER_DEFINED |
| 0x00495170 | CSWCCreatureStats::UpdateAbilityModifiedSkillRanks | CSWCCreatureStats::UpdateAbilityModifiedSkillRanks_2 | USER_DEFINED |
| 0x004951d0 | CExoArrayList<CResRef>::CExoArrayList | CExoArrayList_CResRef_::CExoArrayList | USER_DEFINED |
| 0x00495210 | CExoArrayList<CResRef>::DelIndex | CExoArrayList_CResRef_::DelIndex | USER_DEFINED |
| 0x00495350 | CSWCCreatureStats::ClearKnownSpells | CSWCCreatureStats::ClearKnownSpells_2 | USER_DEFINED |
| 0x004953b3 | CSWCCreatureStats::GetKnownSpell | CSWCCreatureStats::GetKnownSpell_2 | USER_DEFINED |
| 0x00495400 | CExoArrayList<CResRef>::Allocate | CExoArrayList_CResRef_::Allocate | USER_DEFINED |
| 0x00495b40 | CSoundSet::~CSoundSet | CSoundSet::Destructor_2 | USER_DEFINED |
| 0x00496230 | 2060>::CResHelper | 2060_::CResHelper | USER_DEFINED |
| 0x004964f0 | 2060>::scalar | 2060_::scalar | USER_DEFINED |
| 0x00496570 | CSWCMessage::~CSWCMessage | CSWCMessage::Destructor_2 | USER_DEFINED |
| 0x00496bc0 | CSWCMessage::HandleServerToPlayerArea | CSWCMessage::HandleServerToPlayerArea_2 | USER_DEFINED |
| 0x004a4b00 | CSWCMessage::HandleServerToPlayerCreatureUpdate_Update | CSWCMessage::HandleServerToPlayerCreatureUpdate_Update_2 | USER_DEFINED |
| 0x004a76c0 | CSWCMessage::HandleServerToPlayerCreatureUpdate_VisEffect | CSWCMessage::HandleServerToPlayerCreatureUpdate_VisEffect_2 | USER_DEFINED |
| 0x004bf2c0 | CSWCMessage::HandleServerToPlayerUpdate_Update | CSWCMessage::HandleServerToPlayerUpdate_Update_2 | USER_DEFINED |
| 0x004bf390 | CSWCMessage::HandleServerToPlayerAreaOfEffectUpdate_Delete | CSWCMessage::HandleServerToPlayerAreaOfEffectUpdate_Delete_2 | USER_DEFINED |
| 0x004bf710 | CSWCMessage::HandleServerToPlayerUpdate_PlayerInfo | CSWCMessage::HandleServerToPlayerUpdate_PlayerInfo_2 | USER_DEFINED |
| 0x004c0400 | SJournalEntry::SJournalEntry | SJournalEntry::Constructor_2 | USER_DEFINED |
| 0x004c79d0 | CSWCAnimBase::~CSWCAnimBase | CSWCAnimBase::Destructor_2 | USER_DEFINED |
| 0x004c7d90 | CSWCAnimBase::ReplaceTexture | CSWCAnimBase::ReplaceTexture_2 | USER_DEFINED |
| 0x004c7e50 | CSWCAnimBase::ReplaceTexture | CSWCAnimBase::ReplaceTexture_3 | USER_DEFINED |
| 0x004c7f50 | CSWCAnimBase::ReplaceTexture | CSWCAnimBase::ReplaceTexture_4 | USER_DEFINED |
| 0x004c80b0 | CSWCAnimBase::RestoreTexture | CSWCAnimBase::RestoreTexture_2 | USER_DEFINED |
| 0x004c8190 | CSWCAnimBase::SetOverlayAnimation | CSWCAnimBase::SetOverlayAnimation_2 | USER_DEFINED |
| 0x004c90a0 | CSWCAnimBase::SetOrientation | CSWCAnimBase::SetOrientation_2 | USER_DEFINED |
| 0x004c92a0 | CSWCAnimBase::UnloadModel | CSWCAnimBase::UnloadModel_2 | USER_DEFINED |
| 0x004c9560 | CSWCAnimBase::SetEnvMap | CSWCAnimBase::SetEnvMap_2 | USER_DEFINED |
| 0x004cb3a0 | CSWCAnimBase::GetBoundingRect | CSWCAnimBase::GetBoundingRect_2 | USER_DEFINED |
| 0x004ce340 | return_1 | Global::return_1 | USER_DEFINED |
| 0x004ce600 | CSWGuiControl::~CSWGuiControl | CSWGuiControl::Destructor_2 | USER_DEFINED |
| 0x004ce6e0 | CSWGuiNavigable::~CSWGuiNavigable | CSWGuiNavigable::Destructor_2 | USER_DEFINED |
| 0x004cea50 | CGuiInGame::AppendToDialogBuffer | CGuiInGame::AppendToDialogBuffer_2 | USER_DEFINED |
| 0x004dba50 | CGuiInGame::ShowSubItemCreateMenu | CGuiInGame::ShowSubItemCreateMenu_2 | USER_DEFINED |
| 0x004dbaf0 | CGuiInGame::HideSubItemCreateMenu | CGuiInGame::HideSubItemCreateMenu_2 | USER_DEFINED |
| 0x004dbe20 | CGuiInGame::IsLoopingDialogStuntAnimation | CGuiInGame::IsLoopingDialogStuntAnimation_2 | USER_DEFINED |
| 0x004dfe10 | CGuiInGame::IsOverlayDialogAnimation | CGuiInGame::IsOverlayDialogAnimation_2 | USER_DEFINED |
| 0x004e2be0 | CSWCMessage::HandleServerToPlayerSound | CSWCMessage::HandleServerToPlayerSound_2 | USER_DEFINED |
| 0x004e3230 | CAurBehavior_camera_local_Control | Global::CAurBehavior_camera_local_Control | USER_DEFINED |
| 0x004e3250 | CAurBehavior_camera_local_GetType | Global::CAurBehavior_camera_local_GetType | USER_DEFINED |
| 0x004e3260 | CAurBehavior_camera_local_destructor | Global::CAurBehavior_camera_local_destructor | USER_DEFINED |
| 0x004e32c0 | CSWCameraOnAStick::CSWCameraOnAStick | CSWCameraOnAStick::Constructor_2 | USER_DEFINED |
| 0x004e3840 | CSWCameraOnAStick::~CSWCameraOnAStick | CSWCameraOnAStick::Destructor_2 | USER_DEFINED |
| 0x004e65b0 | CSWCameraOnAStick::Control_SetCameraInintalState | CSWCameraOnAStick::Control_SetCameraInintalState_2 | USER_DEFINED |
| 0x004e7f70 | CSWCameraFreeLook::CSWCameraFreeLook | CSWCameraFreeLook::Constructor_2 | USER_DEFINED |
| 0x004e82d0 | CSWCameraFreeLook::~CSWCameraFreeLook | CSWCameraFreeLook::Destructor_2 | USER_DEFINED |
| 0x004e8990 | CSWCameraDeath::~CSWCameraDeath | CSWCameraDeath::Destructor_2 | USER_DEFINED |
| 0x004ea9d0 | SafePointer<CAurObject>::operator= | SafePointer_CAurObject_::operator= | USER_DEFINED |
| 0x004eaf60 | CSWBaseItem_scalar_vector_deleting_destructor | Global::CSWBaseItem_scalar_vector_deleting_destructor | USER_DEFINED |
| 0x004ecbb0 | CSWItem_scalar_deleting_destructor | Global::CSWItem_scalar_deleting_destructor | USER_DEFINED |
| 0x004f3b80 | CSWCAnimBaseHeadWield::ReplaceTexture | CSWCAnimBaseHeadWield::ReplaceTexture_2 | USER_DEFINED |
| 0x004f3bf0 | CSWCAnimBaseHeadWield::ReplaceTexture | CSWCAnimBaseHeadWield::ReplaceTexture_3 | USER_DEFINED |
| 0x004f3c80 | CSWCAnimBaseHeadWield::ReplaceTexture | CSWCAnimBaseHeadWield::ReplaceTexture_4 | USER_DEFINED |
| 0x004f3d30 | CSWCAnimBaseHeadWield::RestoreTexture | CSWCAnimBaseHeadWield::RestoreTexture_2 | USER_DEFINED |
| 0x004f3da0 | CSWCAnimBaseHeadWield::UnloadModel | CSWCAnimBaseHeadWield::UnloadModel_2 | USER_DEFINED |
| 0x004f40d0 | CSWCAnimBaseHeadWield::SetEnvMap | CSWCAnimBaseHeadWield::SetEnvMap_2 | USER_DEFINED |
| 0x004f4c90 | CSWCAnimBaseHeadWield::~CSWCAnimBaseHeadWield | CSWCAnimBaseHeadWield::Destructor_2 | USER_DEFINED |
| 0x004f4cd0 | CSWCAnimBaseHeadWield::~CSWCAnimBaseHeadWield | CSWCAnimBaseHeadWield::Destructor_3 | USER_DEFINED |
| 0x004f4d40 | CSWCAnimBaseHeadWield::virtual_thunk_to_UnloadModel | CSWCAnimBaseHeadWield::virtual_thunk_to_UnloadModel_2 | USER_DEFINED |
| 0x004f4d60 | CSWCAnimBaseHeadWield::virtual_thunk_to_RestoreTexture | CSWCAnimBaseHeadWield::virtual_thunk_to_RestoreTexture_2 | USER_DEFINED |
| 0x004f4e30 | CSWCAnimBaseHeadWield::virtual_thunk_to_SetEnvMap | CSWCAnimBaseHeadWield::virtual_thunk_to_SetEnvMap_2 | USER_DEFINED |
| 0x004f4eb0 | CSWCAnimBaseHeadWield::virtual_thunk_to_ReplaceTexture | CSWCAnimBaseHeadWield::virtual_thunk_to_ReplaceTexture_2 | USER_DEFINED |
| 0x004f4ec0 | CSWCAnimBaseHeadWield::virtual_thunk_to_ReplaceTexture | CSWCAnimBaseHeadWield::virtual_thunk_to_ReplaceTexture_3 | USER_DEFINED |
| 0x004f4ed0 | CSWCAnimBaseHeadWield::virtual_thunk_to_ReplaceTexture | CSWCAnimBaseHeadWield::virtual_thunk_to_ReplaceTexture_4 | USER_DEFINED |
| 0x004f6780 | CSWCAnimBaseWield::~CSWCAnimBaseWield | CSWCAnimBaseWield::Destructor_2 | USER_DEFINED |
| 0x004f6c20 | CSWCAnimBaseHead::ReplaceTexture | CSWCAnimBaseHead::ReplaceTexture_2 | USER_DEFINED |
| 0x004f6d00 | CSWCAnimBaseHead::ReplaceTexture | CSWCAnimBaseHead::ReplaceTexture_3 | USER_DEFINED |
| 0x004f6e30 | CSWCAnimBaseHead::ReplaceTexture | CSWCAnimBaseHead::ReplaceTexture_4 | USER_DEFINED |
| 0x004f6ff0 | CSWCAnimBaseHead::RestoreTexture | CSWCAnimBaseHead::RestoreTexture_2 | USER_DEFINED |
| 0x004f7140 | CSWCAnimBaseHead::UnloadModel | CSWCAnimBaseHead::UnloadModel_2 | USER_DEFINED |
| 0x004f72a0 | CSWCAnimBaseHead::SetEnvMap | CSWCAnimBaseHead::SetEnvMap_2 | USER_DEFINED |
| 0x004f7780 | CSWCAnimBaseHead::virtual_thunk_to_ReplaceTexture | CSWCAnimBaseHead::virtual_thunk_to_ReplaceTexture_2 | USER_DEFINED |
| 0x004f7790 | CSWCAnimBaseHead::virtual_thunk_to_ReplaceTexture | CSWCAnimBaseHead::virtual_thunk_to_ReplaceTexture_3 | USER_DEFINED |
| 0x004f77a0 | CSWCAnimBaseHead::virtual_thunk_to_ReplaceTexture | CSWCAnimBaseHead::virtual_thunk_to_ReplaceTexture_4 | USER_DEFINED |
| 0x004f77e0 | CSWCAnimBaseHead::~CSWCAnimBaseHead | CSWCAnimBaseHead::Destructor_2 | USER_DEFINED |
| 0x004f7840 | CSWCAnimBaseHead::virtual_thunk_to_UnloadModel | CSWCAnimBaseHead::virtual_thunk_to_UnloadModel_2 | USER_DEFINED |
| 0x004f7860 | CSWCAnimBaseHead::virtual_thunk_to_RestoreTexture | CSWCAnimBaseHead::virtual_thunk_to_RestoreTexture_2 | USER_DEFINED |
| 0x004f7900 | CSWCAnimBaseHead::virtual_thunk_to_SetEnvMap | CSWCAnimBaseHead::virtual_thunk_to_SetEnvMap_2 | USER_DEFINED |
| 0x004f7b00 | CSWCAnimBaseTW::~CSWCAnimBaseTW | CSWCAnimBaseTW::Destructor_2 | USER_DEFINED |
| 0x004f7f00 | CSWCAnimBaseTW::ReplaceTexture | CSWCAnimBaseTW::ReplaceTexture_2 | USER_DEFINED |
| 0x004f8090 | CSWCAnimBaseTW::ReplaceTexture | CSWCAnimBaseTW::ReplaceTexture_3 | USER_DEFINED |
| 0x004f82b0 | CSWCAnimBaseTW::ReplaceTexture | CSWCAnimBaseTW::ReplaceTexture_4 | USER_DEFINED |
| 0x004f8590 | CSWCAnimBaseTW::RestoreTexture | CSWCAnimBaseTW::RestoreTexture_2 | USER_DEFINED |
| 0x004f87e0 | CSWCAnimBaseTW::UnloadModel | CSWCAnimBaseTW::UnloadModel_2 | USER_DEFINED |
| 0x004f8a30 | CSWCAnimBaseTW::SetEnvMap | CSWCAnimBaseTW::SetEnvMap_2 | USER_DEFINED |
| 0x005037b0 | CSWCVisualEffectOnObject::GetHardCodedEffectClass | CSWCVisualEffectOnObject::GetHardCodedEffectClass_2 | USER_DEFINED |
| 0x00505b80 | CSWCMessage::SendPlayerToServerLogin_LoginLocalCharacter | CSWCMessage::SendPlayerToServerLogin_LoginLocalCharacter_2 | USER_DEFINED |
| 0x00506820 | MoveNextDoorState | Global::MoveNextDoorState | USER_DEFINED |
| 0x00506a80 | CSWCDoor::~CSWCDoor | CSWCDoor::Destructor_2 | USER_DEFINED |
| 0x0050ab20 | CSWRules::~CSWRules | CSWRules::Destructor_2 | USER_DEFINED |
| 0x0050efa0 | GetAttackBonus | Global::GetAttackBonus | USER_DEFINED |
| 0x0050efd0 | GetFortSaveBonus | Global::GetFortSaveBonus | USER_DEFINED |
| 0x0050f000 | GetRefSaveBonus | Global::GetRefSaveBonus | USER_DEFINED |
| 0x0050f040 | GetWillSaveBonus | Global::GetWillSaveBonus | USER_DEFINED |
| 0x0050f080 | GetSpellGain | Global::GetSpellGain | USER_DEFINED |
| 0x0050f0e0 | GetSpellsKnownPerLevel | Global::GetSpellsKnownPerLevel | USER_DEFINED |
| 0x0050f150 | LoadAttackBonusTable | Global::LoadAttackBonusTable | USER_DEFINED |
| 0x0050f380 | LoadFeatGain | Global::LoadFeatGain | USER_DEFINED |
| 0x0050fb70 | LoadSavingThrowTable | Global::LoadSavingThrowTable | USER_DEFINED |
| 0x0050fe20 | LoadSkillsTable | Global::LoadSkillsTable | USER_DEFINED |
| 0x00510130 | LoadSpellGainTable | Global::LoadSpellGainTable | USER_DEFINED |
| 0x00510570 | LoadSpellKnownTable | Global::LoadSpellKnownTable | USER_DEFINED |
| 0x005108d0 | LoadArmorClassTable | Global::LoadArmorClassTable | USER_DEFINED |
| 0x00510e60 | CSWClass::IsPCGrantedFeat | CSWClass::IsPCGrantedFeat_2 | USER_DEFINED |
| 0x00510ee0 | IsFeatUseable | Global::IsFeatUseable | USER_DEFINED |
| 0x00510f60 | GetRecommendedFeat | Global::GetRecommendedFeat | USER_DEFINED |
| 0x00511020 | GetRecommendedSkill | Global::GetRecommendedSkill | USER_DEFINED |
| 0x005110e0 | IsJedi | Global::IsJedi | USER_DEFINED |
| 0x00511140 | LoadSpellsTable | Global::LoadSpellsTable | USER_DEFINED |
| 0x00511400 | GetRecommendedSpell | Global::GetRecommendedSpell | USER_DEFINED |
| 0x00511580 | GetArmorBonus | Global::GetArmorBonus | USER_DEFINED |
| 0x005115b0 | CSWTlkTable::GetObjectGender | CSWTlkTable::GetObjectGender_2 | USER_DEFINED |
| 0x00513930 | recovered_menu_resref_element_ctor_helper | Global::recovered_menu_resref_element_ctor_helper | USER_DEFINED |
| 0x005172c0 | CSWGuiMainInterfaceChar::DrawEffects | CSWGuiMainInterfaceChar::DrawEffects_2 | USER_DEFINED |
| 0x00517f10 | PopHelpGui | Global::PopHelpGui | USER_DEFINED |
| 0x00518760 | CSWGuiMainInterfaceChar::Show | CSWGuiMainInterfaceChar::Show_2 | USER_DEFINED |
| 0x0051bbd0 | self_return | Global::self_return | USER_DEFINED |
| 0x0051bdc0 | CSWGamepadMenuIos::~CSWGamepadMenuIos | CSWGamepadMenuIos::Destructor_2 | USER_DEFINED |
| 0x0051bef0 | CSWGuiTargetActionMenu::~CSWGuiTargetActionMenu | CSWGuiTargetActionMenu::Destructor_2 | USER_DEFINED |
| 0x0051c290 | CSWGuiProgressBarReveal::~CSWGuiProgressBarReveal | CSWGuiProgressBarReveal::Destructor_2 | USER_DEFINED |
| 0x0051c510 | CSWGuiMainInterface::~CSWGuiMainInterface | CSWGuiMainInterface::Destructor_2 | USER_DEFINED |
| 0x0051f180 | CSWGuiMainInterface::HandleButtonAction | CSWGuiMainInterface::HandleButtonAction_2 | USER_DEFINED |
| 0x0051f400 | CSWGuiMainInterface::UpdateActionQueue | CSWGuiMainInterface::UpdateActionQueue_2 | USER_DEFINED |
| 0x00523460 | CSWGuiMainInterface::HandleButtonAction | CSWGuiMainInterface::HandleButtonAction_3 | USER_DEFINED |
| 0x005253c0 | GetLocalizedCustomString | Global::GetLocalizedCustomString | USER_DEFINED |
| 0x00525450 | GetLocalizedGamepadString | Global::GetLocalizedGamepadString | USER_DEFINED |
| 0x00527ab0 | CSWGuiInGameMenu::~CSWGuiInGameMenu | CSWGuiInGameMenu::Destructor_2 | USER_DEFINED |
| 0x0052a170 | CSWGuiInGameMessages::~CSWGuiInGameMessages | CSWGuiInGameMessages::Destructor_2 | USER_DEFINED |
| 0x0052c53b | SWMSGSetupEffectRow | Global::SWMSGSetupEffectRow | USER_DEFINED |
| 0x0052c960 | CSWGuiMessageBox::CSWGuiMessageBox | CSWGuiMessageBox::Constructor_2 | USER_DEFINED |
| 0x0052d370 | CSWGuiMessageBox::SetMessage | CSWGuiMessageBox::SetMessage_2 | USER_DEFINED |
| 0x0052d4f0 | CSWGuiMessageBox::AddMessageLabel | CSWGuiMessageBox::AddMessageLabel_2 | USER_DEFINED |
| 0x0052ee20 | CSWGuiStatusSummary::~CSWGuiStatusSummary | CSWGuiStatusSummary::Destructor_2 | USER_DEFINED |
| 0x00531b20 | CSWGuiDeathDisplay::~CSWGuiDeathDisplay | CSWGuiDeathDisplay::Destructor_2 | USER_DEFINED |
| 0x005322f0 | AspyrAddStringIcon | Global::AspyrAddStringIcon | USER_DEFINED |
| 0x00532420 | AspyrAddLabelIcon | Global::AspyrAddLabelIcon | USER_DEFINED |
| 0x005324a0 | AspyrAddButtonIcon | Global::AspyrAddButtonIcon | USER_DEFINED |
| 0x005340d0 | CExoArrayList<CSWGuiControl*>::CExoArrayList | CExoArrayList_CSWGuiControl__::CExoArrayList | USER_DEFINED |
| 0x00534110 | CExoArrayList<CSWGuiControl*>::SetSize | CExoArrayList_CSWGuiControl__::SetSize | USER_DEFINED |
| 0x00534170 | CExoArrayList::Add | CExoArrayList::Add_2 | USER_DEFINED |
| 0x005347c0 | CSWGuiPanel::OnBButtonPressed | CSWGuiPanel::OnBButtonPressed_2 | USER_DEFINED |
| 0x00534a80 | CSWCPlaceable::~CSWCPlaceable | CSWCPlaceable::Destructor_2 | USER_DEFINED |
| 0x00537820 | CSWCTrigger::~CSWCTrigger | CSWCTrigger::Destructor_2 | USER_DEFINED |
| 0x0053adf0 | CSWDemoScreen::~CSWDemoScreen | CSWDemoScreen::Destructor_2 | USER_DEFINED |
| 0x0053b090 | UpdateModsButton | Global::UpdateModsButton | USER_DEFINED |
| 0x0053c130 | CSWGuiMainMenu::~CSWGuiMainMenu | CSWGuiMainMenu::Destructor_2 | USER_DEFINED |
| 0x0053e9b0 | CExoArrayList<CExoString>::Remove | CExoArrayList_CExoString_::Remove | USER_DEFINED |
| 0x0053ea00 | CExoArrayList<CSWGuiMainMenu::CErrorMessageData>::CExoArrayList | CExoArrayList_CSWGuiMainMenu__CErrorMessageData_::CExoArrayList | USER_DEFINED |
| 0x0053ea40 | CExoArrayList<CSWGuiMainMenu::CErrorMessageData>::~CExoArrayList | CExoArrayList_CSWGuiMainMenu__CErrorMessageData_::Destructor | USER_DEFINED |
| 0x005400b0 | CSWGuiLoadScreen::~CSWGuiLoadScreen | CSWGuiLoadScreen::Destructor_2 | USER_DEFINED |
| 0x00540530 | OnAnimKeyEvent | Global::OnAnimKeyEvent | USER_DEFINED |
| 0x00540750 | SetInitialHook | Global::SetInitialHook | USER_DEFINED |
| 0x005407d0 | EnumGunAttachPoints | Global::EnumGunAttachPoints | USER_DEFINED |
| 0x00540810 | TrackFollowerPartDie | Global::TrackFollowerPartDie | USER_DEFINED |
| 0x00541e20 | CSWTrackFollower::AttachGuns | CSWTrackFollower::AttachGuns_2 | USER_DEFINED |
| 0x00542fc0 | CSWMiniPlayer::~CSWMiniPlayer | CSWMiniPlayer::Destructor_2 | USER_DEFINED |
| 0x005479d0 | CExoArrayList<SJournalEntry>::CExoArrayList | CExoArrayList_SJournalEntry_::CExoArrayList | USER_DEFINED |
| 0x00547a10 | CExoArrayList<SJournalEntry>::SetSize | CExoArrayList_SJournalEntry_::SetSize | USER_DEFINED |
| 0x00547a90 | CExoArrayList<SJournalEntry>::Add | CExoArrayList_SJournalEntry_::Add | USER_DEFINED |
| 0x00547b60 | CExoArrayList<SJournalEntry>::DelIndex | CExoArrayList_SJournalEntry_::DelIndex | USER_DEFINED |
| 0x00547bc0 | CExoArrayList<CWorldJournalEntry>::CExoArrayList | CExoArrayList_CWorldJournalEntry_::CExoArrayList | USER_DEFINED |
| 0x00547c00 | CExoArrayList<CWorldJournalEntry>::~CExoArrayList | CExoArrayList_CWorldJournalEntry_::Destructor | USER_DEFINED |
| 0x00547c40 | CExoArrayList<CWorldJournalEntry>::SetSize | CExoArrayList_CWorldJournalEntry_::SetSize | USER_DEFINED |
| 0x00547cc0 | CExoArrayList<CWorldJournalEntry>::Add | CExoArrayList_CWorldJournalEntry_::Add | USER_DEFINED |
| 0x00547d90 | CExoArrayList<CWorldJournalEntry>::DelIndex | CExoArrayList_CWorldJournalEntry_::DelIndex | USER_DEFINED |
| 0x00547f20 | CExoArrayList<SJournalEntry>::Allocate | CExoArrayList_SJournalEntry_::Allocate | USER_DEFINED |
| 0x00548090 | CExoArrayList<CWorldJournalEntry>::Allocate | CExoArrayList_CWorldJournalEntry_::Allocate | USER_DEFINED |
| 0x005481e0 | CWorldJournalEntry::CWorldJournalEntry | CWorldJournalEntry::Constructor_2 | USER_DEFINED |
| 0x00548240 | CServerInfo::CServerInfo | CServerInfo::Constructor_2 | USER_DEFINED |
| 0x0054a510 | CSWMessage::~CSWMessage | CSWMessage::Destructor_2 | USER_DEFINED |
| 0x0054c000 | CExoArrayList<float>::CExoArrayList | CExoArrayList_float_::CExoArrayList | USER_DEFINED |
| 0x0054c040 | CExoArrayList<float>::SetSize | CExoArrayList_float_::SetSize | USER_DEFINED |
| 0x0054c0a0 | CExoArrayList<float>::Allocate | CExoArrayList_float_::Allocate | USER_DEFINED |
| 0x0054c5b0 | CSWGuiPazaakCard::~CSWGuiPazaakCard | CSWGuiPazaakCard::Destructor_2 | USER_DEFINED |
| 0x0054e130 | CSWGuiWagerPopup::~CSWGuiWagerPopup | CSWGuiWagerPopup::Destructor_2 | USER_DEFINED |
| 0x0054f270 | CSWGuiTutorial::~CSWGuiTutorial | CSWGuiTutorial::Destructor_2 | USER_DEFINED |
| 0x0054f300 | CSWGuiPazaakStart::~CSWGuiPazaakStart | CSWGuiPazaakStart::Destructor_2 | USER_DEFINED |
| 0x00552940 | CSWGuiPazaakGame::~CSWGuiPazaakGame | CSWGuiPazaakGame::Destructor_2 | USER_DEFINED |
| 0x00558ea0 | CSWGuiInGameCredits::~CSWGuiInGameCredits | CSWGuiInGameCredits::Destructor_2 | USER_DEFINED |
| 0x00559bd0 | CSWBehaviorAttachSound::~CSWBehaviorAttachSound | CSWBehaviorAttachSound::Destructor_2 | USER_DEFINED |
| 0x00559d20 | CSWArea | Global::CSWArea | USER_DEFINED |
| 0x00559f40 | CSWArea::~CSWArea | CSWArea::Destructor_2 | USER_DEFINED |
| 0x0055a020 | InitializeStaticObjects | Global::InitializeStaticObjects | USER_DEFINED |
| 0x0055a0e0 | IntersectLineSegments | Global::IntersectLineSegments | USER_DEFINED |
| 0x0055a670 | SetFog | Global::SetFog | USER_DEFINED |
| 0x0055a6b0 | SetIsNight | Global::SetIsNight | USER_DEFINED |
| 0x0055a6d0 | SetWind | Global::SetWind | USER_DEFINED |
| 0x0055a6f0 | AddStaticBoundingBox | Global::AddStaticBoundingBox | USER_DEFINED |
| 0x0055a8f0 | RemoveStaticBoundingBox | Global::RemoveStaticBoundingBox | USER_DEFINED |
| 0x0055ac90 | CSWCSoundObject::~CSWCSoundObject | CSWCSoundObject::Destructor_2 | USER_DEFINED |
| 0x0055c470 | CExoArrayList<CResRef>::Add | CExoArrayList_CResRef_::Add | USER_DEFINED |
| 0x0055c5c0 | CSWCMapPin::~CSWCMapPin | CSWCMapPin::Destructor_2 | USER_DEFINED |
| 0x0055c630 | AurPartTriggerDestroy | Global::AurPartTriggerDestroy | USER_DEFINED |
| 0x0055cb60 | CSWMiniGame::~CSWMiniGame | CSWMiniGame::Destructor_2 | USER_DEFINED |
| 0x0055eb90 | CSWMiniGame::GetProtoTrack | CSWMiniGame::GetProtoTrack_2 | USER_DEFINED |
| 0x0055f300 | CSWMiniGame::SetPlayer | CSWMiniGame::SetPlayer_2 | USER_DEFINED |
| 0x0055f500 | CSWMiniGame::SetPlayer | CSWMiniGame::SetPlayer_3 | USER_DEFINED |
| 0x00560520 | operator+ | Global::operator+ | USER_DEFINED |
| 0x00561c90 | 2012>::CResHelper | 2012_::CResHelper | USER_DEFINED |
| 0x00561ce0 | 2012>::~CResHelper | 2012_::Destructor | USER_DEFINED |
| 0x00561d90 | 2012>::SetResRef | 2012_::SetResRef | USER_DEFINED |
| 0x00562100 | CAurObject*CSWCRoom::SpawnRoom | CAurObject_CSWCRoom::SpawnRoom | USER_DEFINED |
| 0x00562390 | CSWRoom::~CSWRoom | CSWRoom::Destructor_2 | USER_DEFINED |
| 0x005630b0 | CSWGuiInGameMap::~CSWGuiInGameMap | CSWGuiInGameMap::Destructor_2 | USER_DEFINED |
| 0x00564a70 | CSWGuiMapHider::~CSWGuiMapHider | CSWGuiMapHider::Destructor_2 | USER_DEFINED |
| 0x00566de0 | CSWGuiInGameGalaxyMap::~CSWGuiInGameGalaxyMap | CSWGuiInGameGalaxyMap::Destructor_2 | USER_DEFINED |
| 0x00567c10 | CExoArrayList<int>::CExoArrayList | CExoArrayList_int_::CExoArrayList | USER_DEFINED |
| 0x00567c50 | CExoArrayList<int>::Allocate | CExoArrayList_int_::Allocate | USER_DEFINED |
| 0x00567cf0 | ExpandButtonToFitText | Global::ExpandButtonToFitText | USER_DEFINED |
| 0x00568410 | CSWGuiOptionsMain::~CSWGuiOptionsMain | CSWGuiOptionsMain::Destructor_2 | USER_DEFINED |
| 0x00568ed0 | CSWGuiOptionsFeedback::~CSWGuiOptionsFeedback | CSWGuiOptionsFeedback::Destructor_2 | USER_DEFINED |
| 0x0056ac40 | CSWGuiOptionsGraphics::~CSWGuiOptionsGraphics | CSWGuiOptionsGraphics::Destructor_2 | USER_DEFINED |
| 0x0056c9e0 | CSWGuiOptionsGraphicsAdvanced::~CSWGuiOptionsGraphicsAdvanced | CSWGuiOptionsGraphicsAdvanced::Destructor_2 | USER_DEFINED |
| 0x0056e830 | CSWGuiOptionsResolution::~CSWGuiOptionsResolution | CSWGuiOptionsResolution::Destructor_2 | USER_DEFINED |
| 0x0056fcd0 | CSWGuiOptionsSound::~CSWGuiOptionsSound | CSWGuiOptionsSound::Destructor_2 | USER_DEFINED |
| 0x00570dd0 | CSWGuiOptionsSoundAdvanced::~CSWGuiOptionsSoundAdvanced | CSWGuiOptionsSoundAdvanced::Destructor_2 | USER_DEFINED |
| 0x00572410 | CSWGuiOptionsMouse::~CSWGuiOptionsMouse | CSWGuiOptionsMouse::Destructor_2 | USER_DEFINED |
| 0x00572f70 | GetType | Global::GetType | USER_DEFINED |
| 0x00572f80 | CalculateDesiredLocation | Global::CalculateDesiredLocation | USER_DEFINED |
| 0x00573800 | PutAtDesiredLocation | Global::PutAtDesiredLocation | USER_DEFINED |
| 0x00574b70 | CSWBehaviorCameraDialog::~CSWBehaviorCameraDialog | CSWBehaviorCameraDialog::Destructor_2 | USER_DEFINED |
| 0x00578ad0 | CSWBehaviorCameraDialog::SetShot | CSWBehaviorCameraDialog::SetShot_2 | USER_DEFINED |
| 0x00578e40 | CResSSF::~CResSSF | CResSSF::Destructor_2 | USER_DEFINED |
| 0x005792e0 | CSWCProjectile::~CSWCProjectile | CSWCProjectile::Destructor_2 | USER_DEFINED |
| 0x0057ae60 | CSWCProjectile::UnloadModel | CSWCProjectile::UnloadModel_2 | USER_DEFINED |
| 0x00582200 | CSWCAreaOfEffectObject::~CSWCAreaOfEffectObject | CSWCAreaOfEffectObject::Destructor_2 | USER_DEFINED |
| 0x00584170 | CSWCAreaOfEffectObject::UnloadModel | CSWCAreaOfEffectObject::UnloadModel_2 | USER_DEFINED |
| 0x005851e0 | CSWCAreaOfEffectObject::SetEnvMap | CSWCAreaOfEffectObject::SetEnvMap_2 | USER_DEFINED |
| 0x00588200 | CSWGuiSkillFlowChart::BuildSortedPowerList | CSWGuiSkillFlowChart::BuildSortedPowerList_2 | USER_DEFINED |
| 0x005890a0 | CSWGuiSkillFlowChart::CreateFeatChart | CSWGuiSkillFlowChart::CreateFeatChart_2 | USER_DEFINED |
| 0x0058b740 | CSWGuiPartySelection::~CSWGuiPartySelection | CSWGuiPartySelection::Destructor_2 | USER_DEFINED |
| 0x0058f470 | CSWGuiInGameOptions::~CSWGuiInGameOptions | CSWGuiInGameOptions::Destructor_2 | USER_DEFINED |
| 0x00592000 | CSWGuiButton::~CSWGuiButton | CSWGuiButton::Destructor_2 | USER_DEFINED |
| 0x00592300 | CSWGuiPanel::OnXButtonPressed | CSWGuiPanel::OnXButtonPressed_2 | USER_DEFINED |
| 0x00595290 | CSWGuiInGameAbilities::SetDescription | CSWGuiInGameAbilities::SetDescription_2 | USER_DEFINED |
| 0x00595cc0 | CExoArrayList<CSWGuiSkillStruct>::SetSize | CExoArrayList_CSWGuiSkillStruct_::SetSize | USER_DEFINED |
| 0x00597840 | CSWGuiSlider::~CSWGuiSlider | CSWGuiSlider::Destructor_2 | USER_DEFINED |
| 0x00597870 | CSWGuiInGameCharacter::~CSWGuiInGameCharacter | CSWGuiInGameCharacter::Destructor_2 | USER_DEFINED |
| 0x0059af60 | CSWGuiInGameInventory::~CSWGuiInGameInventory | CSWGuiInGameInventory::Destructor_2 | USER_DEFINED |
| 0x0059cb40 | CSWGuiInGameInventory::SetNextFilter | CSWGuiInGameInventory::SetNextFilter_2 | USER_DEFINED |
| 0x0059d010 | CSWBehaviorCameraDialog::AddCalculatedLineOfAction | CSWBehaviorCameraDialog::AddCalculatedLineOfAction_2 | USER_DEFINED |
| 0x0059e5a0 | CSWGuiInGameEquip::~CSWGuiInGameEquip | CSWGuiInGameEquip::Destructor_2 | USER_DEFINED |
| 0x005a4420 | CSWGuiInGameEquip::EquipItem | CSWGuiInGameEquip::EquipItem_2 | USER_DEFINED |
| 0x005a4ab0 | CSWGuiInGameItemEntry::~CSWGuiInGameItemEntry | CSWGuiInGameItemEntry::Destructor_2 | USER_DEFINED |
| 0x005a61c0 | CSWGuiContainer::~CSWGuiContainer | CSWGuiContainer::Destructor_2 | USER_DEFINED |
| 0x005a73f0 | CSWGuiStoreItemEntry::~CSWGuiStoreItemEntry | CSWGuiStoreItemEntry::Destructor_2 | USER_DEFINED |
| 0x005ad1f0 | CSWGuiInGameAreaTransition::SetTransitionObject | CSWGuiInGameAreaTransition::SetTransitionObject_2 | USER_DEFINED |
| 0x005ad480 | CSWGuiInGameSoloModeQuery::~CSWGuiInGameSoloModeQuery | CSWGuiInGameSoloModeQuery::Destructor_2 | USER_DEFINED |
| 0x005ae5d0 | CSWGuiDialog::~CSWGuiDialog | CSWGuiDialog::Destructor_2 | USER_DEFINED |
| 0x005b06f0 | CSWGuiDialogCinematic::~CSWGuiDialogCinematic | CSWGuiDialogCinematic::Destructor_2 | USER_DEFINED |
| 0x005b17e0 | CSWGuiDialogComputer::~CSWGuiDialogComputer | CSWGuiDialogComputer::Destructor_2 | USER_DEFINED |
| 0x005b2260 | CSWGuiDialogComputerCamera::~CSWGuiDialogComputerCamera | CSWGuiDialogComputerCamera::Destructor_2 | USER_DEFINED |
| 0x005b3800 | CSWGuiCreateItemSubMenu::~CSWGuiCreateItemSubMenu | CSWGuiCreateItemSubMenu::Destructor_2 | USER_DEFINED |
| 0x005b7250 | CSWGuiUpgradeSelection::~CSWGuiUpgradeSelection | CSWGuiUpgradeSelection::Destructor_2 | USER_DEFINED |
| 0x005b8880 | CSWGuiUpgradeItemSelect::~CSWGuiUpgradeItemSelect | CSWGuiUpgradeItemSelect::Destructor_2 | USER_DEFINED |
| 0x005c2590 | CSWGuiCreateItem::~CSWGuiCreateItem | CSWGuiCreateItem::Destructor_2 | USER_DEFINED |
| 0x005c7ec0 | CSWGuiCreateMedicalItem::~CSWGuiCreateMedicalItem | CSWGuiCreateMedicalItem::Destructor_2 | USER_DEFINED |
| 0x005ca1a0 | CSWGuiCreateMedicalItem::OnPanelRemoved | CSWGuiCreateMedicalItem::OnPanelRemoved_2 | USER_DEFINED |
| 0x005d1cd0 | LoadScreenUpdate | Global::LoadScreenUpdate | USER_DEFINED |
| 0x005d5140 | CSWGuiSaveLoadEntry::CSWGuiSaveLoadEntry | CSWGuiSaveLoadEntry::Constructor_2 | USER_DEFINED |
| 0x005d7d00 | CSWGuiEditbox::~CSWGuiEditbox | CSWGuiEditbox::Destructor_2 | USER_DEFINED |
| 0x005d7df0 | CSWGuiSaveNamePanel::~CSWGuiSaveNamePanel | CSWGuiSaveNamePanel::Destructor_2 | USER_DEFINED |
| 0x005d8840 | CExoArrayList<CSWGuiControl*>::Insert | CExoArrayList_CSWGuiControl__::Insert | USER_DEFINED |
| 0x005d88a0 | CExoArrayList<CSWGuiKeyMapButton*>::SetSize | CExoArrayList_CSWGuiKeyMapButton__::SetSize | USER_DEFINED |
| 0x005d88d0 | CExoArrayList<CExoString>::Insert | CExoArrayList_CExoString_::Insert | USER_DEFINED |
| 0x005d9610 | operator/ | Global::operator/ | USER_DEFINED |
| 0x005d9830 | GetAccelerationVector | Global::GetAccelerationVector | USER_DEFINED |
| 0x005d9a50 | CSWCAnimBaseVisualEffect::~CSWCAnimBaseVisualEffect | CSWCAnimBaseVisualEffect::Destructor_2 | USER_DEFINED |
| 0x005d9d90 | CSWCAnimBaseDoor::~CSWCAnimBaseDoor | CSWCAnimBaseDoor::Destructor_2 | USER_DEFINED |
| 0x005da5e0 | CSWCAnimBasePlaceable::~CSWCAnimBasePlaceable | CSWCAnimBasePlaceable::Destructor_2 | USER_DEFINED |
| 0x005dafe0 | CSWCAnimBaseTrigger | Global::CSWCAnimBaseTrigger | USER_DEFINED |
| 0x005db040 | CSWCAnimBaseTrigger::~CSWCAnimBaseTrigger | CSWCAnimBaseTrigger::Destructor_2 | USER_DEFINED |
| 0x005db0a0 | LoadModel | Global::LoadModel | USER_DEFINED |
| 0x005db1f0 | ServerToClientAnimation | Global::ServerToClientAnimation | USER_DEFINED |
| 0x005db2b0 | FireGunCallback | Global::FireGunCallback | USER_DEFINED |
| 0x005dc280 | GunDie | Global::GunDie | USER_DEFINED |
| 0x005de6b0 | CSWGuiClassSelection::~CSWGuiClassSelection | CSWGuiClassSelection::Destructor_2 | USER_DEFINED |
| 0x005e1850 | CSWGuiTitleMusic::~CSWGuiTitleMusic | CSWGuiTitleMusic::Destructor_2 | USER_DEFINED |
| 0x005e1f8d | CSWGuiTitleMusic::PopulateSongList | CSWGuiTitleMusic::PopulateSongList_2 | USER_DEFINED |
| 0x005e27d0 | CExoArrayList<CMusicItemInfo>::CExoArrayList | CExoArrayList_CMusicItemInfo_::CExoArrayList | USER_DEFINED |
| 0x005e2810 | CExoArrayList<CMusicItemInfo>::~CExoArrayList | CExoArrayList_CMusicItemInfo_::Destructor | USER_DEFINED |
| 0x005e2850 | CExoArrayList<CMusicItemInfo>::Add | CExoArrayList_CMusicItemInfo_::Add | USER_DEFINED |
| 0x005e2910 | CExoArrayList<CMusicItemInfo>::ClearAndOptionalSetSize | CExoArrayList_CMusicItemInfo_::ClearAndOptionalSetSize | USER_DEFINED |
| 0x005e29b0 | CExoArrayList<CMusicItemInfo>::Allocate | CExoArrayList_CMusicItemInfo_::Allocate | USER_DEFINED |
| 0x005e2b10 | CExoArrayList<CMusicItemInfo>::SetSize | CExoArrayList_CMusicItemInfo_::SetSize | USER_DEFINED |
| 0x005e4200 | static_int_swp_player_has_tiebreaker_CPazaakPlayer_ref | Global::static_int_swp_player_has_tiebreaker_CPazaakPlayer_ref | USER_DEFINED |
| 0x005e5240 | CSWGuiInGameGameplay::~CSWGuiInGameGameplay | CSWGuiInGameGameplay::Destructor_2 | USER_DEFINED |
| 0x005e69c0 | CSWGuiInGameAutoPause::~CSWGuiInGameAutoPause | CSWGuiInGameAutoPause::Destructor_2 | USER_DEFINED |
| 0x005e7280 | CSWCAnimBaseProjectile::~CSWCAnimBaseProjectile | CSWCAnimBaseProjectile::Destructor_2 | USER_DEFINED |
| 0x005e86c0 | CSWGuiLevelUpCharGen::~CSWGuiLevelUpCharGen | CSWGuiLevelUpCharGen::Destructor_2 | USER_DEFINED |
| 0x005ed370 | CSWGuiMainCharGen::~CSWGuiMainCharGen | CSWGuiMainCharGen::Destructor_2 | USER_DEFINED |
| 0x005f3990 | CSWGuiCustomPanel::~CSWGuiCustomPanel | CSWGuiCustomPanel::Destructor_2 | USER_DEFINED |
| 0x005f5e90 | CSWGuiQuickOrCustomPanel::~CSWGuiQuickOrCustomPanel | CSWGuiQuickOrCustomPanel::Destructor_2 | USER_DEFINED |
| 0x005f6f00 | CSWGuiPowersLevelUp::~CSWGuiPowersLevelUp | CSWGuiPowersLevelUp::Destructor_2 | USER_DEFINED |
| 0x005f7380 | CSWGuiPowersLevelUp::ShowGranted | CSWGuiPowersLevelUp::ShowGranted_2 | USER_DEFINED |
| 0x005fd330 | CSWGuiSkillsCharGen::~CSWGuiSkillsCharGen | CSWGuiSkillsCharGen::Destructor_2 | USER_DEFINED |
| 0x006010f0 | CSWGuiAbilitiesCharGen::~CSWGuiAbilitiesCharGen | CSWGuiAbilitiesCharGen::Destructor_2 | USER_DEFINED |
| 0x00603a30 | CSWGuiPortraitCharGen::~CSWGuiPortraitCharGen | CSWGuiPortraitCharGen::Destructor_2 | USER_DEFINED |
| 0x00605680 | CExoString::CExoString | CExoString::CStrConstructor, CExoString::Constructor_2 | USER_DEFINED |
| 0x00605700 | CExoString::CExoString | CExoString::Constructor_3 | USER_DEFINED |
| 0x00605790 | CExoString::CExoString | CExoString::CStrLenConstructor, CExoString::Constructor_4 | USER_DEFINED |
| 0x00605810 | CExoString::CExoString | CExoString::Constructor_5 | USER_DEFINED |
| 0x006059e0 | CExoString::operator= | CExoString::operator=_2 | USER_DEFINED |
| 0x00605b70 | CExoString::operator== | CExoString::operator==_2 | USER_DEFINED |
| 0x00605c70 | CExoString::operator!= | CExoString::operator!=_2 | USER_DEFINED |
| 0x006062a0 | CExoString::Find | CExoString::Find_2 | USER_DEFINED |
| 0x00607070 | CExoString::CompareNoCase | CExoString::CompareNoCase_2 | USER_DEFINED |
| 0x006073e0 | CExoLocString::CExoLocString | CExoLocString::Constructor_2 | USER_DEFINED |
| 0x006076c0 | CExoLocString::GetString | CExoLocString::GetString_2 | USER_DEFINED |
| 0x006077c0 | CExoLocString::GetString | CExoLocString::GetString_3 | USER_DEFINED |
| 0x00607990 | CExoLocString::GetStringLoc | CExoLocString::GetStringLoc_2 | USER_DEFINED |
| 0x006084b0 | GetGameVersionInfo | Global::GetGameVersionInfo | USER_DEFINED |
| 0x0060b670 | CExoArrayList<T>::~CExoArrayList | CExoArrayList_T_::Destructor | USER_DEFINED |
| 0x0060b6a0 | CExoArrayList<_FILETIME>::Insert | CExoArrayList__FILETIME_::Insert | USER_DEFINED |
| 0x0060ba50 | CExoFile::Read | CExoFile::Read_2 | USER_DEFINED |
| 0x0060bb10 | CExoFile::Write | CExoFile::Write_2 | USER_DEFINED |
| 0x0060bb30 | CExoFile::Write | CExoFile::Write_3 | USER_DEFINED |
| 0x0060bcd0 | CExoFile::SeekEnd | CExoFile::SeekEnd_2 | USER_DEFINED |
| 0x0060c2f0 | EXOMALLOC | Global::EXOMALLOC | USER_DEFINED |
| 0x0060c310 | EXOFREE | Global::EXOFREE | USER_DEFINED |
| 0x0060dbf0 | CExoLocStringInternal::GetString | CExoLocStringInternal::GetString_2 | USER_DEFINED |
| 0x0060f700 | AsyncReadThreadProc | Global::AsyncReadThreadProc | USER_DEFINED |
| 0x00610220 | CExoFileInternal::Read | CExoFileInternal::Read_2 | USER_DEFINED |
| 0x00610320 | CExoFileInternal::Write | CExoFileInternal::Write_2 | USER_DEFINED |
| 0x00611d10 | CExoInputInternal::~CExoInputInternal | CExoInputInternal::Destructor_2 | USER_DEFINED |
| 0x006129e0 | CExoInputEventDescDetailed::CExoInputEventDescDetailed | CExoInputEventDescDetailed::Constructor_2 | USER_DEFINED |
| 0x006133b0 | return_void | Global::return_void | USER_DEFINED |
| 0x00613fb2 | CExoInputInternal::GetEvents | CExoInputInternal::GetEvents_2 | USER_DEFINED |
| 0x00614520 | CExoInputInternal::GetEvents | CExoInputInternal::GetEvents_3 | USER_DEFINED |
| 0x00614678 | CExoInputInternal::GetEvents | CExoInputInternal::GetEvents_4 | USER_DEFINED |
| 0x006154e0 | CExoInputInternal::BufferEvent | CExoInputInternal::BufferEvent_2 | USER_DEFINED |
| 0x006155f0 | IsCursorCurrentlyVisible | Global::IsCursorCurrentlyVisible | USER_DEFINED |
| 0x00615600 | IsCursorInsideWindow | Global::IsCursorInsideWindow | USER_DEFINED |
| 0x00615680 | UpdateCusorVisible | Global::UpdateCusorVisible | USER_DEFINED |
| 0x00616b40 | CExoInput::GetDisplayCharacterForKey | CExoInput::GetDisplayCharacterForKey_2 | USER_DEFINED |
| 0x006172a0 | CountJoysticksExt | Global::CountJoysticksExt | USER_DEFINED |
| 0x006172c0 | InitJoysticksExt | Global::InitJoysticksExt | USER_DEFINED |
| 0x0061a140 | CResRef::CResRef | CResRef::Constructor_2 | USER_DEFINED |
| 0x0061a160 | CResRef::CResRef | CResRef::Constructor_3 | USER_DEFINED |
| 0x0061a190 | CResRef::CResRef | CResRef::Constructor_4 | USER_DEFINED |
| 0x0061a1c0 | CResRef::CResRef | CResRef::Constructor_5 | USER_DEFINED |
| 0x0061a1f0 | CResRef::CResRef | CResRef::Constructor_6 | USER_DEFINED |
| 0x0061a2d0 | CResRef::CopyToString | CResRef::CopyToString_2 | USER_DEFINED |
| 0x0061a3f0 | CResRef::operator!= | CResRef::operator!=_2 | USER_DEFINED |
| 0x0061a420 | CResRef::operator!= | CResRef::operator!=_3 | USER_DEFINED |
| 0x0061a4c0 | CResRef::operator== | CResRef::operator==_2 | USER_DEFINED |
| 0x0061a570 | CResRef::operator== | CResRef::operator==_3 | USER_DEFINED |
| 0x0061a660 | CResRef::operator= | CResRef::operator=_2 | USER_DEFINED |
| 0x0061a710 | CResRef::operator= | CResRef::operator=_3 | USER_DEFINED |
| 0x0061a800 | CResRef::operator= | CResRef::operator=_4 | USER_DEFINED |
| 0x0061ab00 | CRes::~CRes | CRes::Destructor_2 | USER_DEFINED |
| 0x0061ac50 | AsyncRimLoadThreadProc | Global::AsyncRimLoadThreadProc | USER_DEFINED |
| 0x0061b890 | CExoResMan::Exists | CExoResMan::Exists_2 | USER_DEFINED |
| 0x0061c280 | CExoResMan::GetResOfType | CExoResMan::GetResOfType_2 | USER_DEFINED |
| 0x0061c2d0 | *CExoResMan::GetResObject | _CExoResMan::GetResObject | USER_DEFINED |
| 0x006206e0 | CExoResMan::FreeResourceData | CExoResMan::FreeResourceData_2 | USER_DEFINED |
| 0x00620740 | CExoResMan::CacheResource | CExoResMan::CacheResource_2 | USER_DEFINED |
| 0x006207c0 | CExoLinkedListInternal::Remove | CExoLinkedListInternal::Remove_2 | USER_DEFINED |
| 0x00620920 | C2DA::~C2DA | C2DA::Destructor_2 | USER_DEFINED |
| 0x00620b50 | C2DA::GetCExoStringEntry | C2DA::GetCExoStringEntry_2 | USER_DEFINED |
| 0x00620c70 | C2DA::GetCExoStringEntry | C2DA::GetCExoStringEntry_3 | USER_DEFINED |
| 0x00620d80 | C2DA::GetCExoStringEntry | C2DA::GetCExoStringEntry_4 | USER_DEFINED |
| 0x00620e90 | GetFLOATEntry | Global::GetFLOATEntry | USER_DEFINED |
| 0x00621110 | C2DA::GetFLOATEntry | C2DA::GetFLOATEntry_2 | USER_DEFINED |
| 0x00621250 | C2DA::GetFLOATEntry | C2DA::GetFLOATEntry_3 | USER_DEFINED |
| 0x00621350 | GetINTEntry | Global::GetINTEntry | USER_DEFINED |
| 0x00621790 | C2DA::GetINTEntry | C2DA::GetINTEntry_2 | USER_DEFINED |
| 0x006219b0 | C2DA::GetINTEntry | C2DA::GetINTEntry_3 | USER_DEFINED |
| 0x00623130 | CResGFF::CResGFF | CResGFF::Constructor_2 | USER_DEFINED |
| 0x00623310 | CResGFF::~CResGFF | CResGFF::Destructor_2 | USER_DEFINED |
| 0x00623b60 | CResGFF::GetField | CResGFF::GetField_2 | USER_DEFINED |
| 0x00624840 | CResGFF::GetFieldCount | CResGFF::GetFieldCount_2 | USER_DEFINED |
| 0x00626f10 | CLIP::~CLIP | CLIP::Destructor_2 | USER_DEFINED |
| 0x00629690 | CLYT::LoadLayout | CLYT::LoadLayout_2 | USER_DEFINED |
| 0x0062bb40 | CERFFile::WriteResource | CERFFile::WriteResource_2 | USER_DEFINED |
| 0x00630390 | CExoKeyTable::FindKey | CExoKeyTable::FindKey_2 | USER_DEFINED |
| 0x00630990 | CExoPackedFile::~CExoPackedFile | CExoPackedFile::Destructor_2 | USER_DEFINED |
| 0x00630a70 | CExoResFile::~CExoResFile | CExoResFile::Destructor_2 | USER_DEFINED |
| 0x006314e0 | CExoEncapsulatedFile::~CExoEncapsulatedFile | CExoEncapsulatedFile::Destructor_2 | USER_DEFINED |
| 0x00632f80 | CExoResourceImageFile::~CExoResourceImageFile | CExoResourceImageFile::Destructor_2 | USER_DEFINED |
| 0x00633a00 | CRes2DA::~CRes2DA | CRes2DA::Destructor_2 | USER_DEFINED |
| 0x00633bb0 | CResLIP::~CResLIP | CResLIP::Destructor_2 | USER_DEFINED |
| 0x006392a0 | CExoSoundInternal::GetSliderType | CExoSoundInternal::GetSliderType_2 | USER_DEFINED |
| 0x006394c0 | CExoSoundInternal::GetPauseable | CExoSoundInternal::GetPauseable_2 | USER_DEFINED |
| 0x00639a10 | CExoLinkedList<T>::Contains | CExoLinkedList_T_::Contains | USER_DEFINED |
| 0x00639b30 | CExoSoundSource::CExoSoundSource | CExoSoundSource::Constructor_2 | USER_DEFINED |
| 0x0063d120 | CExoStreamingSoundSourceInternal_volume_scale_eligibility_helper | Global::CExoStreamingSoundSourceInternal_volume_scale_eligibility_helper | USER_DEFINED |
| 0x0063dc50 | CExoSoundSourceInternal_loaded_resource_release_helper | Global::CExoSoundSourceInternal_loaded_resource_release_helper | USER_DEFINED |
| 0x0063dc90 | CExoSoundSourceInternal::~CExoSoundSourceInternal | CExoSoundSourceInternal::Destructor_2 | USER_DEFINED |
| 0x0063efc0 | CExoSoundSourceInternal_volume_scale_eligibility_helper | Global::CExoSoundSourceInternal_volume_scale_eligibility_helper | USER_DEFINED |
| 0x0063f5d0 | CExoSoundSourceInternal_fixed_variance_frequency_helper | Global::CExoSoundSourceInternal_fixed_variance_frequency_helper | USER_DEFINED |
| 0x0063f660 | CResHelper<CResWave,4>::CResHelper | CResHelper_CResWave_4_::CResHelper | USER_DEFINED |
| 0x0063f6b0 | CResHelper<CResWave,4>::~CResHelper | CResHelper_CResWave_4_::Destructor | USER_DEFINED |
| 0x0063f760 | CResHelper<CResWave,4>::deleting | CResHelper_CResWave_4_::deleting | USER_DEFINED |
| 0x00640120 | CNetLayer::EndServerMode | CNetLayer::EndServerMode_2 | USER_DEFINED |
| 0x00640420 | CNetLayer::NonWindowMessages | CNetLayer::NonWindowMessages_2 | USER_DEFINED |
| 0x006415a0 | 2009>::~CResHelper | 2009_::Destructor | USER_DEFINED |
| 0x00642387 | CVirtualMachineStack::LoadStack | CVirtualMachineStack::LoadStack_2 | USER_DEFINED |
| 0x0064240a | CVirtualMachineStack::LoadStack | CVirtualMachineStack::LoadStack_3 | USER_DEFINED |
| 0x006424e0 | CVirtualMachineStack::ClearStack | CVirtualMachineStack::ClearStack_2 | USER_DEFINED |
| 0x00642a20 | CVirtualMachineInternal::~CVirtualMachineInternal | CVirtualMachineInternal::Destructor_2 | USER_DEFINED |
| 0x006430ee | CVirtualMachineInternal::ExecuteCode | CVirtualMachineInternal::ExecuteCode_2 | USER_DEFINED |
| 0x00643793 | CVirtualMachineInternal::ExecuteCode | CVirtualMachineInternal::ExecuteCode_3 | USER_DEFINED |
| 0x006445ae | CVirtualMachineInternal::ExecuteCode | CVirtualMachineInternal::ExecuteCode_4 | USER_DEFINED |
| 0x00645160 | CVirtualMachineInternal::ExecuteCode | CVirtualMachineInternal::ExecuteCode_5 | USER_DEFINED |
| 0x006452b0 | CVirtualMachineInternal::ReadScriptFile | CVirtualMachineInternal::ReadScriptFile_2 | USER_DEFINED |
| 0x00645549 | CVirtualMachineInternal::RunScript | CVirtualMachineInternal::RunScript_2 | USER_DEFINED |
| 0x00646af0 | CResHelper<CResNCS,2010>::CResHelper | CResHelper_CResNCS_2010_::CResHelper | USER_DEFINED |
| 0x00646b40 | CResHelper<CResNCS,2010>::~CResHelper | CResHelper_CResNCS_2010_::Destructor | USER_DEFINED |
| 0x00646bf0 | CResHelper<CResNCS,2010>::SetResRef | CResHelper_CResNCS_2010_::SetResRef | USER_DEFINED |
| 0x00646db0 | CResHelper<CResNCS,2010>::~CResHelper | CResHelper_CResNCS_2010_::Destructor_2 | USER_DEFINED |
| 0x00646e50 | CResNCS::~CResNCS | CResNCS::Destructor_2 | USER_DEFINED |
| 0x0064dd70 | LogEventAdd | Global::LogEventAdd | USER_DEFINED |
| 0x0064e4c0 | LogEventAddGetObject | Global::LogEventAddGetObject | USER_DEFINED |
| 0x00657710 | CSWSCreature::IsInTotalDefense | CSWSCreature::IsInTotalDefense_2 | USER_DEFINED |
| 0x0065aca0 | rand_wincompatible | Global::rand_wincompatible | USER_DEFINED |
| 0x00663910 | CSWSItem::~CSWSItem | CSWSItem::Destructor_2 | USER_DEFINED |
| 0x0067c0f0 | CSWSCreature::~CSWSCreature | CSWSCreature::Destructor_2 | USER_DEFINED |
| 0x00696e80 | CSWSCreature::Rest | CSWSCreature::Rest_2 | USER_DEFINED |
| 0x00698df0 | CSWSCreature::UpdateAppearanceForEquippedItems | CSWSCreature::UpdateAppearanceForEquippedItems_2 | USER_DEFINED |
| 0x0069f120 | CSWSCreature::PacifyCreature | CSWSCreature::PacifyCreature_2 | USER_DEFINED |
| 0x006a5190 | CLoopingVisualEffect::GetIsBeam | CLoopingVisualEffect::GetIsBeam_2 | USER_DEFINED |
| 0x006a5d30 | CSWSObject::~CSWSObject | CSWSObject::Destructor_2 | USER_DEFINED |
| 0x006ad730 | Global_thunk_ClientExoApp_IsSoundPlayingInDialog | Global::Global_thunk_ClientExoApp_IsSoundPlayingInDialog | USER_DEFINED |
| 0x006ad7e0 | CSWSObject::RemoveObjectFromDialog | CSWSObject::RemoveObjectFromDialog_2 | USER_DEFINED |
| 0x006aed70 | CSWSObject::BroadcastSafeProjectile | CSWSObject::BroadcastSafeProjectile_2 | USER_DEFINED |
| 0x006b16f0 | CExoArrayList<CSWSScriptVar>::CExoArrayList | CExoArrayList_CSWSScriptVar_::CExoArrayList | USER_DEFINED |
| 0x006b1730 | CExoArrayList<CSWSScriptVar>::Allocate | CExoArrayList_CSWSScriptVar_::Allocate | USER_DEFINED |
| 0x006c7820 | CSWSCreature::AIActionCheckMoveToPoint | CSWSCreature::AIActionCheckMoveToPoint_2 | USER_DEFINED |
| 0x006dc300 | CSWSCreature::GetCreatureReputation | CSWSCreature::GetCreatureReputation_2 | USER_DEFINED |
| 0x006e02d0 | CExoArrayList::Add | CExoArrayList::Add_3 | USER_DEFINED |
| 0x006f5220 | CSWSCreatureStats::DecrementFeatRemainingUses | CSWSCreatureStats::DecrementFeatRemainingUses_2 | USER_DEFINED |
| 0x006f88b0 | CSWSCreatureStats::GetFeat | CSWSCreatureStats::GetFeat_2 | USER_DEFINED |
| 0x00700170 | CSWPartyTable::AddNPC | CSWPartyTable::AddNPC_2 | USER_DEFINED |
| 0x007002b0 | CSWPartyTable::AddPUP | CSWPartyTable::AddPUP_2 | USER_DEFINED |
| 0x00700be0 | CSWPartyTable::GetIsMember | CSWPartyTable::GetIsMember_2 | USER_DEFINED |
| 0x00703960 | CSWPartyTable::GivePlotXP | CSWPartyTable::GivePlotXP_2 | USER_DEFINED |
| 0x00705710 | CSWSArea::GetRoom | CSWSArea::GetRoom_2 | USER_DEFINED |
| 0x00713490 | QueryTimer | Global::QueryTimer | USER_DEFINED |
| 0x00715890 | CSWRoomSurfaceMesh::ComputeHeight | CSWRoomSurfaceMesh::ComputeHeight_2 | USER_DEFINED |
| 0x00716310 | GetObstructionFactor | Global::GetObstructionFactor | USER_DEFINED |
| 0x00716b80 | CSWSArea::~CSWSArea | CSWSArea::Destructor_2 | USER_DEFINED |
| 0x00720dc0 | CSWSArea::CSWSArea | CSWSArea::Constructor_2 | USER_DEFINED |
| 0x00720f20 | CSWSArea::~CSWSArea | CSWSArea::Destructor_3 | USER_DEFINED |
| 0x00722ee0 | CSWSArea::GetClientArea | CSWSArea::GetClientArea_2 | USER_DEFINED |
| 0x00723e00 | CSWCollisionMesh::GetVertex | CSWCollisionMesh::GetVertex_2 | USER_DEFINED |
| 0x00728270 | SetTriangleCount | Global::SetTriangleCount | USER_DEFINED |
| 0x00728440 | WorldToLocal | Global::WorldToLocal | USER_DEFINED |
| 0x00728c60 | AddModuleOverrideResourceDirectories | Global::AddModuleOverrideResourceDirectories | USER_DEFINED |
| 0x00728d40 | RemoveModuleOverrideResourceDirectories | Global::RemoveModuleOverrideResourceDirectories | USER_DEFINED |
| 0x007294d0 | CSWSModule::~CSWSModule | CSWSModule::Destructor_2 | USER_DEFINED |
| 0x007336c0 | CSWCMessage::ReadCExoLocStringClient | CSWCMessage::ReadCExoLocStringClient_2 | USER_DEFINED |
| 0x007337d0 | CExoArrayList<_FILETIME>::CExoArrayList | CExoArrayList__FILETIME_::CExoArrayList | USER_DEFINED |
| 0x00733b00 | CExoLinkedListInternal::AddTail | CExoLinkedListInternal::AddTail_2 | USER_DEFINED |
| 0x00733d10 | CExoArrayList<_FILETIME>::Allocate | CExoArrayList__FILETIME_::Allocate | USER_DEFINED |
| 0x00734500 | CSWSRoom::ComputeHeight | CSWSRoom::ComputeHeight_2 | USER_DEFINED |
| 0x00734780 | CGameEffect::CGameEffect | CGameEffect::Constructor_2 | USER_DEFINED |
| 0x00736240 | CSWSEffectListHandler::~CSWSEffectListHandler | CSWSEffectListHandler::Destructor_2 | USER_DEFINED |
| 0x007457b0 | sub_007457B0_tail_wing_return_one | Global::sub_007457B0_tail_wing_return_one | USER_DEFINED |
| 0x0074d530 | CSWSDialog::~CSWSDialog | CSWSDialog::Destructor_2 | USER_DEFINED |
| 0x00750ca0 | CSWSDialog::CheckScript | CSWSDialog::CheckScript_2 | USER_DEFINED |
| 0x00758980 | CSWSDialog::GetStartEntryOneLiner | CSWSDialog::GetStartEntryOneLiner_2 | USER_DEFINED |
| 0x00758c90 | CSWSDialog::SetDialogDelay | CSWSDialog::SetDialogDelay_2 | USER_DEFINED |
| 0x00759e20 | CSWSPlayer::~CSWSPlayer | CSWSPlayer::Destructor_2 | USER_DEFINED |
| 0x0075d370 | CSWSTrigger::~CSWSTrigger | CSWSTrigger::Destructor_2 | USER_DEFINED |
| 0x00762440 | CSWSDoor::~CSWSDoor | CSWSDoor::Destructor_2 | USER_DEFINED |
| 0x00768760 | CSWSDoor::GetNearestActionPoint | CSWSDoor::GetNearestActionPoint_2 | USER_DEFINED |
| 0x007695a0 | CSWSPlaceable::~CSWSPlaceable | CSWSPlaceable::Destructor_2 | USER_DEFINED |
| 0x00777c00 | CExoLinkedList<CAuroraModel>::AddBefore | CExoLinkedList_CAuroraModel_::AddBefore | USER_DEFINED |
| 0x0077a8f0 | CExoArrayList<CSWSPlayerJournalQuestUpdates>::Add | CExoArrayList_CSWSPlayerJournalQuestUpdates_::Add | USER_DEFINED |
| 0x0077a9b0 | CSWCCMessageData::CSWCCMessageData | CSWCCMessageData::Constructor_2 | USER_DEFINED |
| 0x0077aa40 | CSWCCMessageData::~CSWCCMessageData | CSWCCMessageData::Destructor_2 | USER_DEFINED |
| 0x0077b500 | CSWVirtualMachineCommands::~CSWVirtualMachineCommands | CSWVirtualMachineCommands::Destructor_2 | USER_DEFINED |
| 0x0077eef0 | ExecuteCommandActionBarkString | Global::ExecuteCommandActionBarkString | USER_DEFINED |
| 0x0077f170 | ExecuteCommandActionExchangeItem | Global::ExecuteCommandActionExchangeItem | USER_DEFINED |
| 0x0077f260 | ExecuteCommandActionFollowObject | Global::ExecuteCommandActionFollowObject | USER_DEFINED |
| 0x0077f5a0 | ExecuteCommandActionFollowLeader | Global::ExecuteCommandActionFollowLeader | USER_DEFINED |
| 0x0077f640 | ExecuteCommandActionInteractObject | Global::ExecuteCommandActionInteractObject | USER_DEFINED |
| 0x0077f6d0 | ExecuteCommandActionJumpToObject | Global::ExecuteCommandActionJumpToObject | USER_DEFINED |
| 0x0077f810 | ExecuteCommandActionJumpToPoint | Global::ExecuteCommandActionJumpToPoint | USER_DEFINED |
| 0x0077fac0 | ExecuteCommandActionLockActions | Global::ExecuteCommandActionLockActions | USER_DEFINED |
| 0x0077fb70 | ExecuteCommandActionMoveAwayFromLocation | Global::ExecuteCommandActionMoveAwayFromLocation | USER_DEFINED |
| 0x0077ff30 | ExecuteCommandActionPauseConversation | Global::ExecuteCommandActionPauseConversation | USER_DEFINED |
| 0x00780100 | ExecuteCommandActionRandomWalk | Global::ExecuteCommandActionRandomWalk | USER_DEFINED |
| 0x00780240 | ExecuteCommandActionResumeConversation | Global::ExecuteCommandActionResumeConversation | USER_DEFINED |
| 0x007802f0 | ExecuteCommandActionStartConversation | Global::ExecuteCommandActionStartConversation | USER_DEFINED |
| 0x00780c20 | ExecuteCommandSetCreatureAILevel | Global::ExecuteCommandSetCreatureAILevel | USER_DEFINED |
| 0x00780d50 | ExecuteCommandChangeObjectAppearance | Global::ExecuteCommandChangeObjectAppearance | USER_DEFINED |
| 0x00780de0 | ExecuteCommandResetCreatureAILevel | Global::ExecuteCommandResetCreatureAILevel | USER_DEFINED |
| 0x00780e80 | ExecuteCommandCreatureFlourishWeapon | Global::ExecuteCommandCreatureFlourishWeapon | USER_DEFINED |
| 0x00781130 | ExecuteCommandActionUseFeat | Global::ExecuteCommandActionUseFeat | USER_DEFINED |
| 0x00781230 | ExecuteCommandActionUseSkill | Global::ExecuteCommandActionUseSkill | USER_DEFINED |
| 0x007813a0 | ExecuteCommandActionUseTalent | Global::ExecuteCommandActionUseTalent | USER_DEFINED |
| 0x00781580 | ExecuteCommandAddAvailableNPCByObject | Global::ExecuteCommandAddAvailableNPCByObject | USER_DEFINED |
| 0x00781620 | ExecuteCommandAddAvailableNPCByTemplate | Global::ExecuteCommandAddAvailableNPCByTemplate | USER_DEFINED |
| 0x00781740 | ExecuteCommandAddJournalQuestEntry | Global::ExecuteCommandAddJournalQuestEntry | USER_DEFINED |
| 0x00781a10 | ExecuteCommandAddJournalWorldEntry | Global::ExecuteCommandAddJournalWorldEntry | USER_DEFINED |
| 0x00781c40 | ExecuteCommandAddJournalWorldEntryStrref | Global::ExecuteCommandAddJournalWorldEntryStrref | USER_DEFINED |
| 0x00781d80 | ExecuteCommandAddMultiClass | Global::ExecuteCommandAddMultiClass | USER_DEFINED |
| 0x00781f70 | ExecuteCommandAddPartyMember | Global::ExecuteCommandAddPartyMember | USER_DEFINED |
| 0x00782060 | ExecuteCommandAddToParty | Global::ExecuteCommandAddToParty | USER_DEFINED |
| 0x00782130 | ExecuteCommandAdjustAlignment | Global::ExecuteCommandAdjustAlignment | USER_DEFINED |
| 0x00782790 | ExecuteCommandAdjustReputation | Global::ExecuteCommandAdjustReputation | USER_DEFINED |
| 0x00782890 | ExecuteCommandAmbientSound | Global::ExecuteCommandAmbientSound | USER_DEFINED |
| 0x00782ab0 | ExecuteCommandApplyEffectAtPoint | Global::ExecuteCommandApplyEffectAtPoint | USER_DEFINED |
| 0x00782d60 | ExecuteCommandApplyEffectOnObject | Global::ExecuteCommandApplyEffectOnObject | USER_DEFINED |
| 0x00783310 | ExecuteCommandBarkString | Global::ExecuteCommandBarkString | USER_DEFINED |
| 0x00783550 | ExecuteCommandBeginConversation | Global::ExecuteCommandBeginConversation | USER_DEFINED |
| 0x00783dc0 | ExecuteCommandCastSpell | Global::ExecuteCommandCastSpell | USER_DEFINED |
| 0x00784680 | ExecuteCommandChangeFaction | Global::ExecuteCommandChangeFaction | USER_DEFINED |
| 0x00784910 | ExecuteCommandClearAllActions | Global::ExecuteCommandClearAllActions | USER_DEFINED |
| 0x00784990 | ExecuteCommandCloseDoor | Global::ExecuteCommandCloseDoor | USER_DEFINED |
| 0x00784a60 | ExecuteCommandCreateItemOnObject | Global::ExecuteCommandCreateItemOnObject | USER_DEFINED |
| 0x00784f70 | ExecuteCommandCreateObject | Global::ExecuteCommandCreateObject | USER_DEFINED |
| 0x00785a90 | ExecuteCommandDeleteJournalWorldAllEntries | Global::ExecuteCommandDeleteJournalWorldAllEntries | USER_DEFINED |
| 0x00785b60 | ExecuteCommandDeleteJournalWorldEntry | Global::ExecuteCommandDeleteJournalWorldEntry | USER_DEFINED |
| 0x00785c50 | ExecuteCommandDeleteJournalWorldEntryStrref | Global::ExecuteCommandDeleteJournalWorldEntryStrref | USER_DEFINED |
| 0x00785d40 | ExecuteCommandDestroyObject | Global::ExecuteCommandDestroyObject | USER_DEFINED |
| 0x007860c0 | ExecuteCommandDisplayFeedBackText | Global::ExecuteCommandDisplayFeedBackText | USER_DEFINED |
| 0x00786270 | ExecuteCommandDistanceConversions | Global::ExecuteCommandDistanceConversions | USER_DEFINED |
| 0x007862f0 | ExecuteCommandDoDoorAction | Global::ExecuteCommandDoDoorAction | USER_DEFINED |
| 0x00786680 | ExecuteCommandDoSinglePlayerAutoSave | Global::ExecuteCommandDoSinglePlayerAutoSave | USER_DEFINED |
| 0x007866b0 | ExecuteCommandDoTouchAttack | Global::ExecuteCommandDoTouchAttack | USER_DEFINED |
| 0x00786c40 | ExecuteCommandEffectAbilityDecrease | Global::ExecuteCommandEffectAbilityDecrease | USER_DEFINED |
| 0x00786e00 | ExecuteCommandEffectAbilityIncrease | Global::ExecuteCommandEffectAbilityIncrease | USER_DEFINED |
| 0x00786fc0 | ExecuteCommandEffectACDecrease | Global::ExecuteCommandEffectACDecrease | USER_DEFINED |
| 0x007871e0 | ExecuteCommandEffectACIncrease | Global::ExecuteCommandEffectACIncrease | USER_DEFINED |
| 0x00787400 | ExecuteCommandEffectAreaEffect | Global::ExecuteCommandEffectAreaEffect | USER_DEFINED |
| 0x00787700 | ExecuteCommandEffectAttackDecrease | Global::ExecuteCommandEffectAttackDecrease | USER_DEFINED |
| 0x007878e0 | ExecuteCommandEffectAttackIncrease | Global::ExecuteCommandEffectAttackIncrease | USER_DEFINED |
| 0x00787ac0 | ExecuteCommandEffectBeam | Global::ExecuteCommandEffectBeam | USER_DEFINED |
| 0x00787cb0 | ExecuteCommandEffectBlind | Global::ExecuteCommandEffectBlind | USER_DEFINED |
| 0x00787e10 | ExecuteCommandEffectCrush | Global::ExecuteCommandEffectCrush | USER_DEFINED |
| 0x00787f10 | ExecuteCommandEffectComplex | Global::ExecuteCommandEffectComplex | USER_DEFINED |
| 0x00788850 | ExecuteCommandEffectConcealment | Global::ExecuteCommandEffectConcealment | USER_DEFINED |
| 0x007889d0 | ExecuteCommandEffectDamage | Global::ExecuteCommandEffectDamage | USER_DEFINED |
| 0x00788c70 | ExecuteCommandEffectDamageDecrease | Global::ExecuteCommandEffectDamageDecrease | USER_DEFINED |
| 0x00788e60 | ExecuteCommandEffectDamageImmunityDecrease | Global::ExecuteCommandEffectDamageImmunityDecrease | USER_DEFINED |
| 0x00789030 | ExecuteCommandEffectDamageImmunityIncrease | Global::ExecuteCommandEffectDamageImmunityIncrease | USER_DEFINED |
| 0x00789200 | ExecuteCommandEffectDamageIncrease | Global::ExecuteCommandEffectDamageIncrease | USER_DEFINED |
| 0x007893f0 | ExecuteCommandEffectDamageReduction | Global::ExecuteCommandEffectDamageReduction | USER_DEFINED |
| 0x00789600 | ExecuteCommandEffectDamageResistance | Global::ExecuteCommandEffectDamageResistance | USER_DEFINED |
| 0x007897f0 | ExecuteCommandEffectForceShield | Global::ExecuteCommandEffectForceShield | USER_DEFINED |
| 0x00789930 | ExecuteCommandEffectDamageShield | Global::ExecuteCommandEffectDamageShield | USER_DEFINED |
| 0x00789b10 | ExecuteCommandEffectFactionModifier | Global::ExecuteCommandEffectFactionModifier | USER_DEFINED |
| 0x00789c80 | ExecuteCommandEffectForceBody | Global::ExecuteCommandEffectForceBody | USER_DEFINED |
| 0x00789df0 | ExecuteCommandEffectForceSight | Global::ExecuteCommandEffectForceSight | USER_DEFINED |
| 0x00789f30 | ExecuteCommandEffectFPRegenModifier | Global::ExecuteCommandEffectFPRegenModifier | USER_DEFINED |
| 0x0078a0a0 | ExecuteCommandEffectVPRegenModifier | Global::ExecuteCommandEffectVPRegenModifier | USER_DEFINED |
| 0x0078a210 | ExecuteCommandEffectFury | Global::ExecuteCommandEffectFury | USER_DEFINED |
| 0x0078a350 | ExecuteCommandEffectForceDrain | Global::ExecuteCommandEffectForceDrain | USER_DEFINED |
| 0x0078a500 | ExecuteCommandEffectHeal | Global::ExecuteCommandEffectHeal | USER_DEFINED |
| 0x0078a6c0 | ExecuteCommandEffectHitPointChangeWhenDying | Global::ExecuteCommandEffectHitPointChangeWhenDying | USER_DEFINED |
| 0x0078a840 | ExecuteCommandEffectImmunity | Global::ExecuteCommandEffectImmunity | USER_DEFINED |
| 0x0078a9f0 | ExecuteCommandEffectInvisibility | Global::ExecuteCommandEffectInvisibility | USER_DEFINED |
| 0x0078ab80 | ExecuteCommandEffectLinkEffects | Global::ExecuteCommandEffectLinkEffects | USER_DEFINED |
| 0x0078acf0 | ExecuteCommandEffectMissChance | Global::ExecuteCommandEffectMissChance | USER_DEFINED |
| 0x0078ae60 | ExecuteCommandEffectModifyAttacks | Global::ExecuteCommandEffectModifyAttacks | USER_DEFINED |
| 0x0078afc0 | ExecuteCommandEffectMovementSpeedDecrease | Global::ExecuteCommandEffectMovementSpeedDecrease | USER_DEFINED |
| 0x0078b150 | ExecuteCommandEffectMovementSpeedIncrease | Global::ExecuteCommandEffectMovementSpeedIncrease | USER_DEFINED |
| 0x0078b2e0 | ExecuteCommandEffectPoison | Global::ExecuteCommandEffectPoison | USER_DEFINED |
| 0x0078b420 | ExecuteCommandEffectResurrection | Global::ExecuteCommandEffectResurrection | USER_DEFINED |
| 0x0078b5a0 | ExecuteCommandEffectSavingThrowDecrease | Global::ExecuteCommandEffectSavingThrowDecrease | USER_DEFINED |
| 0x0078b7b0 | ExecuteCommandEffectSavingThrowIncrease | Global::ExecuteCommandEffectSavingThrowIncrease | USER_DEFINED |
| 0x0078b9c0 | ExecuteCommandEffectSeeInvisible | Global::ExecuteCommandEffectSeeInvisible | USER_DEFINED |
| 0x0078bae0 | ExecuteCommandEffectSkillDecrease | Global::ExecuteCommandEffectSkillDecrease | USER_DEFINED |
| 0x0078bca0 | ExecuteCommandEffectSkillIncrease | Global::ExecuteCommandEffectSkillIncrease | USER_DEFINED |
| 0x0078be60 | ExecuteCommandEffectSpellImmunity | Global::ExecuteCommandEffectSpellImmunity | USER_DEFINED |
| 0x0078bfe0 | ExecuteCommandEffectSpellLevelAbsorption | Global::ExecuteCommandEffectSpellLevelAbsorption | USER_DEFINED |
| 0x0078c1c0 | ExecuteCommandEffectForceResistanceDecrease | Global::ExecuteCommandEffectForceResistanceDecrease | USER_DEFINED |
| 0x0078c300 | ExecuteCommandEffectForceResistanceIncrease | Global::ExecuteCommandEffectForceResistanceIncrease | USER_DEFINED |
| 0x0078c5f0 | ExecuteCommandEffectTemporaryHP | Global::ExecuteCommandEffectTemporaryHP | USER_DEFINED |
| 0x0078c7a0 | ExecuteCommandEffectTrueSeeing | Global::ExecuteCommandEffectTrueSeeing | USER_DEFINED |
| 0x0078c8c0 | ExecuteCommandEffectVisualEffect | Global::ExecuteCommandEffectVisualEffect | USER_DEFINED |
| 0x0078cac0 | ExecuteCommandEffectDroidStun | Global::ExecuteCommandEffectDroidStun | USER_DEFINED |
| 0x0078cbc0 | ExecuteCommandEffectForceJump | Global::ExecuteCommandEffectForceJump | USER_DEFINED |
| 0x0078cd30 | ExecuteCommandEffectForcePushed | Global::ExecuteCommandEffectForcePushed | USER_DEFINED |
| 0x0078ce40 | ExecuteCommandEffectForcePushTargeted | Global::ExecuteCommandEffectForcePushTargeted | USER_DEFINED |
| 0x0078d030 | ExecuteCommandEffectWhirlwind | Global::ExecuteCommandEffectWhirlwind | USER_DEFINED |
| 0x0078d130 | ExecuteCommandEffectBlasterDeflectionIncrease | Global::ExecuteCommandEffectBlasterDeflectionIncrease | USER_DEFINED |
| 0x0078d250 | ExecuteCommandEffectBlasterDeflectionDecrease | Global::ExecuteCommandEffectBlasterDeflectionDecrease | USER_DEFINED |
| 0x0078d370 | ExecuteCommandEffectHorrified | Global::ExecuteCommandEffectHorrified | USER_DEFINED |
| 0x0078d470 | ExecuteCommandEffectDamageForcePoints | Global::ExecuteCommandEffectDamageForcePoints | USER_DEFINED |
| 0x0078d590 | ExecuteCommandEffectHealForcePoints | Global::ExecuteCommandEffectHealForcePoints | USER_DEFINED |
| 0x0078d6b0 | ExecuteCommandEffectTemporaryFP | Global::ExecuteCommandEffectTemporaryFP | USER_DEFINED |
| 0x0078d7d0 | ExecuteCommandEffectChoke | Global::ExecuteCommandEffectChoke | USER_DEFINED |
| 0x0078d8d0 | ExecuteCommandEffectBodyFuel | Global::ExecuteCommandEffectBodyFuel | USER_DEFINED |
| 0x0078d9c0 | ExecuteCommandEffectPsychicStatic | Global::ExecuteCommandEffectPsychicStatic | USER_DEFINED |
| 0x0078dab0 | ExecuteCommandEffectLightsaberThrow | Global::ExecuteCommandEffectLightsaberThrow | USER_DEFINED |
| 0x0078dc70 | ExecuteCommandEndGame | Global::ExecuteCommandEndGame | USER_DEFINED |
| 0x0078dce0 | ExecuteCommandEquipItem | Global::ExecuteCommandEquipItem | USER_DEFINED |
| 0x0078de70 | ExecuteCommandEventActivateItem | Global::ExecuteCommandEventActivateItem | USER_DEFINED |
| 0x0078e130 | ExecuteCommandEventConversation | Global::ExecuteCommandEventConversation | USER_DEFINED |
| 0x0078e220 | ExecuteCommandEventSpellCastAt | Global::ExecuteCommandEventSpellCastAt | USER_DEFINED |
| 0x0078e490 | ExecuteCommandEventUserDefined | Global::ExecuteCommandEventUserDefined | USER_DEFINED |
| 0x0078e630 | ExecuteCommandExecuteScript | Global::ExecuteCommandExecuteScript | USER_DEFINED |
| 0x0078e770 | ExecuteCommandExportAllCharacters | Global::ExecuteCommandExportAllCharacters | USER_DEFINED |
| 0x0078e7a0 | ExecuteCommandFindSubString | Global::ExecuteCommandFindSubString | USER_DEFINED |
| 0x0078e8f0 | ExecuteCommandFloatingText | Global::ExecuteCommandFloatingText | USER_DEFINED |
| 0x0078ec20 | ExecuteCommandFloatToString | Global::ExecuteCommandFloatToString | USER_DEFINED |
| 0x0078edf0 | ExecuteCommandForceHeartbeat | Global::ExecuteCommandForceHeartbeat | USER_DEFINED |
| 0x0078eed0 | ExecuteCommandGetAbilityScore | Global::ExecuteCommandGetAbilityScore | USER_DEFINED |
| 0x0078f0e0 | ExecuteCommandGetAbilityModifier | Global::ExecuteCommandGetAbilityModifier | USER_DEFINED |
| 0x0078f270 | ExecuteCommandGetAC | Global::ExecuteCommandGetAC | USER_DEFINED |
| 0x0078f3b0 | ExecuteCommandGetAlignment | Global::ExecuteCommandGetAlignment | USER_DEFINED |
| 0x0078f500 | ExecuteCommandGetAOEObjectCreator | Global::ExecuteCommandGetAOEObjectCreator | USER_DEFINED |
| 0x0078f5c0 | ExecuteCommandGetAppearanceType | Global::ExecuteCommandGetAppearanceType | USER_DEFINED |
| 0x0078f650 | ExecuteCommandGetArea | Global::ExecuteCommandGetArea | USER_DEFINED |
| 0x0078f7c0 | ExecuteCommandGetAreaUnescapable | Global::ExecuteCommandGetAreaUnescapable | USER_DEFINED |
| 0x0078f830 | ExecuteCommandGetAttackTarget | Global::ExecuteCommandGetAttackTarget | USER_DEFINED |
| 0x0078f8b0 | ExecuteCommandGetAttemptedAttackTarget | Global::ExecuteCommandGetAttemptedAttackTarget | USER_DEFINED |
| 0x0078f920 | ExecuteCommandGetAttemptedSpellTarget | Global::ExecuteCommandGetAttemptedSpellTarget | USER_DEFINED |
| 0x0078f990 | ExecuteCommandGetAttemptedMovementTarget | Global::ExecuteCommandGetAttemptedMovementTarget | USER_DEFINED |
| 0x0078fa00 | ExecuteCommandGetSpellTarget | Global::ExecuteCommandGetSpellTarget | USER_DEFINED |
| 0x0078fa80 | ExecuteCommandGetBaseItemType | Global::ExecuteCommandGetBaseItemType | USER_DEFINED |
| 0x0078fb00 | ExecuteCommandGetBattleTrack | Global::ExecuteCommandGetBattleTrack | USER_DEFINED |
| 0x0078fb90 | ExecuteCommandGetBlockingDoor | Global::ExecuteCommandGetBlockingDoor | USER_DEFINED |
| 0x0078fc00 | ExecuteCommandGetCasterLevel | Global::ExecuteCommandGetCasterLevel | USER_DEFINED |
| 0x0078feb0 | ExecuteCommandGetChallengeRating | Global::ExecuteCommandGetChallengeRating | USER_DEFINED |
| 0x0078ff50 | ExecuteCommandGetClassInformation | Global::ExecuteCommandGetClassInformation | USER_DEFINED |
| 0x00790200 | ExecuteCommandGetCommandable | Global::ExecuteCommandGetCommandable | USER_DEFINED |
| 0x007902a0 | ExecuteCommandGetCreatureHasTalent | Global::ExecuteCommandGetCreatureHasTalent | USER_DEFINED |
| 0x007903a0 | ExecuteCommandGetCreatureSize | Global::ExecuteCommandGetCreatureSize | USER_DEFINED |
| 0x00790420 | ExecuteCommandGetCreatureTalent | Global::ExecuteCommandGetCreatureTalent | USER_DEFINED |
| 0x007906e0 | ExecuteCommandGetCurrentAction | Global::ExecuteCommandGetCurrentAction | USER_DEFINED |
| 0x00790810 | ExecuteCommandGetDamageDealtByType | Global::ExecuteCommandGetDamageDealtByType | USER_DEFINED |
| 0x00790900 | ExecuteCommandGetDayTrack | Global::ExecuteCommandGetDayTrack | USER_DEFINED |
| 0x00790990 | ExecuteCommandGetDistanceBetween | Global::ExecuteCommandGetDistanceBetween | USER_DEFINED |
| 0x00790a80 | ExecuteCommandGetDistanceBetween2D | Global::ExecuteCommandGetDistanceBetween2D | USER_DEFINED |
| 0x00790ba0 | ExecuteCommandGetDistanceBetweenLocations | Global::ExecuteCommandGetDistanceBetweenLocations | USER_DEFINED |
| 0x00790db0 | ExecuteCommandGetDistanceBetweenLocations2D | Global::ExecuteCommandGetDistanceBetweenLocations2D | USER_DEFINED |
| 0x00791000 | ExecuteCommandGetDistanceToObject | Global::ExecuteCommandGetDistanceToObject | USER_DEFINED |
| 0x007911a0 | ExecuteCommandGetDistanceToObject2D | Global::ExecuteCommandGetDistanceToObject2D | USER_DEFINED |
| 0x00791350 | ExecuteCommandGetEffect | Global::ExecuteCommandGetEffect | USER_DEFINED |
| 0x007916e0 | ExecuteCommandGetEffectCreator | Global::ExecuteCommandGetEffectCreator | USER_DEFINED |
| 0x00791790 | ExecuteCommandGetEffectDurationType | Global::ExecuteCommandGetEffectDurationType | USER_DEFINED |
| 0x00791830 | ExecuteCommandGetEffectSpellId | Global::ExecuteCommandGetEffectSpellId | USER_DEFINED |
| 0x007918d0 | ExecuteCommandGetEffectSubType | Global::ExecuteCommandGetEffectSubType | USER_DEFINED |
| 0x00791970 | ExecuteCommandGetEffectType | Global::ExecuteCommandGetEffectType | USER_DEFINED |
| 0x00791a10 | ExecuteCommandGetEmotions | Global::ExecuteCommandGetEmotions | USER_DEFINED |
| 0x00791bf0 | ExecuteCommandGetEncounterData | Global::ExecuteCommandGetEncounterData | USER_DEFINED |
| 0x00791cd0 | ExecuteCommandGetEnteringObject | Global::ExecuteCommandGetEnteringObject | USER_DEFINED |
| 0x00791e90 | ExecuteCommandGetFacing | Global::ExecuteCommandGetFacing | USER_DEFINED |
| 0x00791ff0 | ExecuteCommandGetFactionAverageGoodEvilAlignment | Global::ExecuteCommandGetFactionAverageGoodEvilAlignment | USER_DEFINED |
| 0x007920d0 | ExecuteCommandGetFactionAverageLevel | Global::ExecuteCommandGetFactionAverageLevel | USER_DEFINED |
| 0x007921b0 | ExecuteCommandGetFactionAverageReputation | Global::ExecuteCommandGetFactionAverageReputation | USER_DEFINED |
| 0x007922a0 | ExecuteCommandGetFactionAverageXP | Global::ExecuteCommandGetFactionAverageXP | USER_DEFINED |
| 0x00792380 | ExecuteCommandGetFactionBestAC | Global::ExecuteCommandGetFactionBestAC | USER_DEFINED |
| 0x00792490 | ExecuteCommandGetFactionEqual | Global::ExecuteCommandGetFactionEqual | USER_DEFINED |
| 0x00792550 | ExecuteCommandGetFactionGold | Global::ExecuteCommandGetFactionGold | USER_DEFINED |
| 0x00792630 | ExecuteCommandGetFactionLeastDamagedMember | Global::ExecuteCommandGetFactionLeastDamagedMember | USER_DEFINED |
| 0x00792740 | ExecuteCommandGetFactionLeader | Global::ExecuteCommandGetFactionLeader | USER_DEFINED |
| 0x007927d0 | ExecuteCommandGetFactionMember | Global::ExecuteCommandGetFactionMember | USER_DEFINED |
| 0x00792960 | ExecuteCommandGetFactionMostDamagedMember | Global::ExecuteCommandGetFactionMostDamagedMember | USER_DEFINED |
| 0x00792a70 | ExecuteCommandGetFactionMostFrequentClass | Global::ExecuteCommandGetFactionMostFrequentClass | USER_DEFINED |
| 0x00792b50 | ExecuteCommandGetFactionStrongestMember | Global::ExecuteCommandGetFactionStrongestMember | USER_DEFINED |
| 0x00792c60 | ExecuteCommandGetFactionWeakestMember | Global::ExecuteCommandGetFactionWeakestMember | USER_DEFINED |
| 0x00792d70 | ExecuteCommandGetFactionWorstAC | Global::ExecuteCommandGetFactionWorstAC | USER_DEFINED |
| 0x00792e80 | ExecuteCommandGetFortitudeSavingThrow | Global::ExecuteCommandGetFortitudeSavingThrow | USER_DEFINED |
| 0x00792fd0 | ExecuteCommandGetGameDifficulty | Global::ExecuteCommandGetGameDifficulty | USER_DEFINED |
| 0x00793030 | ExecuteCommandGetGlobalLocation | Global::ExecuteCommandGetGlobalLocation | USER_DEFINED |
| 0x00793150 | ExecuteCommandGetGlobalNumber | Global::ExecuteCommandGetGlobalNumber | USER_DEFINED |
| 0x00793290 | ExecuteCommandGetGlobalString | Global::ExecuteCommandGetGlobalString | USER_DEFINED |
| 0x007933b0 | ExecuteCommandGetGoingToBeAttackedBy | Global::ExecuteCommandGetGoingToBeAttackedBy | USER_DEFINED |
| 0x00793430 | ExecuteCommandGetGold | Global::ExecuteCommandGetGold | USER_DEFINED |
| 0x00793530 | ExecuteCommandGetGoldPieceValue | Global::ExecuteCommandGetGoldPieceValue | USER_DEFINED |
| 0x007935e0 | ExecuteCommandGetChemical | Global::ExecuteCommandGetChemical | USER_DEFINED |
| 0x007936e0 | ExecuteCommandGetChemicalPieceValue | Global::ExecuteCommandGetChemicalPieceValue | USER_DEFINED |
| 0x00793790 | ExecuteCommandGetHasFeat | Global::ExecuteCommandGetHasFeat | USER_DEFINED |
| 0x007938b0 | ExecuteCommandGetHasFeatEffect | Global::ExecuteCommandGetHasFeatEffect | USER_DEFINED |
| 0x00793970 | ExecuteCommandGetHasInventory | Global::ExecuteCommandGetHasInventory | USER_DEFINED |
| 0x00793a80 | ExecuteCommandGetHasSkill | Global::ExecuteCommandGetHasSkill | USER_DEFINED |
| 0x00793b70 | ExecuteCommandGetHasSpell | Global::ExecuteCommandGetHasSpell | USER_DEFINED |
| 0x00793c50 | ExecuteCommandGetHasSpellEffect | Global::ExecuteCommandGetHasSpellEffect | USER_DEFINED |
| 0x00793d10 | ExecuteCommandGetHitDice | Global::ExecuteCommandGetHitDice | USER_DEFINED |
| 0x00793df0 | ExecuteCommandGetHitpoints | Global::ExecuteCommandGetHitpoints | USER_DEFINED |
| 0x00793f80 | ExecuteCommandGetCurrentForcePoints | Global::ExecuteCommandGetCurrentForcePoints | USER_DEFINED |
| 0x00794030 | ExecuteCommandGetMaxForcePoints | Global::ExecuteCommandGetMaxForcePoints | USER_DEFINED |
| 0x007940b0 | ExecuteCommandGetIdentified | Global::ExecuteCommandGetIdentified | USER_DEFINED |
| 0x00794130 | ExecuteCommandGetInPersistentObject | Global::ExecuteCommandGetInPersistentObject | USER_DEFINED |
| 0x00794e20 | ExecuteCommandGetInventoryDisturbItem | Global::ExecuteCommandGetInventoryDisturbItem | USER_DEFINED |
| 0x00794ea0 | ExecuteCommandGetInventoryDisturbType | Global::ExecuteCommandGetInventoryDisturbType | USER_DEFINED |
| 0x00794f20 | ExecuteCommandGetIsDawn | Global::ExecuteCommandGetIsDawn | USER_DEFINED |
| 0x00794f90 | ExecuteCommandGetIsDay | Global::ExecuteCommandGetIsDay | USER_DEFINED |
| 0x00795000 | ExecuteCommandGetIsDoorActionPossible | Global::ExecuteCommandGetIsDoorActionPossible | USER_DEFINED |
| 0x007953a0 | ExecuteCommandGetIsDusk | Global::ExecuteCommandGetIsDusk | USER_DEFINED |
| 0x00795410 | ExecuteCommandGetIsEncounterCreature | Global::ExecuteCommandGetIsEncounterCreature | USER_DEFINED |
| 0x007954a0 | ExecuteCommandGetIsImmune | Global::ExecuteCommandGetIsImmune | USER_DEFINED |
| 0x007955e0 | ExecuteCommandGetIsInCombat | Global::ExecuteCommandGetIsInCombat | USER_DEFINED |
| 0x007956b0 | ExecuteCommandGetIsNight | Global::ExecuteCommandGetIsNight | USER_DEFINED |
| 0x00795720 | ExecuteCommandGetIsOpen | Global::ExecuteCommandGetIsOpen | USER_DEFINED |
| 0x00795810 | ExecuteCommandGetIsRacialTypePlayable | Global::ExecuteCommandGetIsRacialTypePlayable | USER_DEFINED |
| 0x00795930 | ExecuteCommandGetIsTalentValid | Global::ExecuteCommandGetIsTalentValid | USER_DEFINED |
| 0x007959e0 | ExecuteCommandGetIsMined | Global::ExecuteCommandGetIsMined | USER_DEFINED |
| 0x00795af0 | ExecuteCommandGetIsWeaponEffective | Global::ExecuteCommandGetIsWeaponEffective | USER_DEFINED |
| 0x00795b90 | ExecuteCommandGetItemACValue | Global::ExecuteCommandGetItemACValue | USER_DEFINED |
| 0x00795c80 | ExecuteCommandGetItemHasItemProperty | Global::ExecuteCommandGetItemHasItemProperty | USER_DEFINED |
| 0x00795e10 | ExecuteCommandGetItemInInventory | Global::ExecuteCommandGetItemInInventory | USER_DEFINED |
| 0x00796500 | ExecuteCommandGetItemInSlot | Global::ExecuteCommandGetItemInSlot | USER_DEFINED |
| 0x00796630 | ExecuteCommandGetItemPossessedBy | Global::ExecuteCommandGetItemPossessedBy | USER_DEFINED |
| 0x00796930 | ExecuteCommandGetItemPossessor | Global::ExecuteCommandGetItemPossessor | USER_DEFINED |
| 0x00796a30 | ExecuteCommandGetJournalEntry | Global::ExecuteCommandGetJournalEntry | USER_DEFINED |
| 0x00796bc0 | ExecuteCommandGetJournalQuestExperience | Global::ExecuteCommandGetJournalQuestExperience | USER_DEFINED |
| 0x00796e60 | ExecuteCommandGetLastAttacker | Global::ExecuteCommandGetLastAttacker | USER_DEFINED |
| 0x00796f10 | ExecuteCommandGetLastHostileTarget | Global::ExecuteCommandGetLastHostileTarget | USER_DEFINED |
| 0x00796f90 | ExecuteCommandGetLastAttackAction | Global::ExecuteCommandGetLastAttackAction | USER_DEFINED |
| 0x00797010 | ExecuteCommandGetLastAttackMode | Global::ExecuteCommandGetLastAttackMode | USER_DEFINED |
| 0x00797150 | ExecuteCommandGetLastAttackResult | Global::ExecuteCommandGetLastAttackResult | USER_DEFINED |
| 0x007971d0 | ExecuteCommandGetWasForcePowerSuccessful | Global::ExecuteCommandGetWasForcePowerSuccessful | USER_DEFINED |
| 0x00797250 | ExecuteCommandGetLastAttackType | Global::ExecuteCommandGetLastAttackType | USER_DEFINED |
| 0x00797340 | ExecuteCommandGetLastClosedBy | Global::ExecuteCommandGetLastClosedBy | USER_DEFINED |
| 0x00797400 | ExecuteCommandGetLastCombatFeatUsed | Global::ExecuteCommandGetLastCombatFeatUsed | USER_DEFINED |
| 0x00797480 | ExecuteCommandGetLastDamager | Global::ExecuteCommandGetLastDamager | USER_DEFINED |
| 0x00797510 | ExecuteCommandGetLastDisarmed | Global::ExecuteCommandGetLastDisarmed | USER_DEFINED |
| 0x00797610 | ExecuteCommandGetLastDisturbed | Global::ExecuteCommandGetLastDisturbed | USER_DEFINED |
| 0x007976f0 | ExecuteCommandGetLastForcePowerUsed | Global::ExecuteCommandGetLastForcePowerUsed | USER_DEFINED |
| 0x00797770 | ExecuteCommandGetLastHostileActor | Global::ExecuteCommandGetLastHostileActor | USER_DEFINED |
| 0x007979b0 | ExecuteCommandGetLastKiller | Global::ExecuteCommandGetLastKiller | USER_DEFINED |
| 0x00797a40 | ExecuteCommandGetLastLocked | Global::ExecuteCommandGetLastLocked | USER_DEFINED |
| 0x00797b20 | ExecuteCommandGetLastOpenedBy | Global::ExecuteCommandGetLastOpenedBy | USER_DEFINED |
| 0x00797be0 | ExecuteCommandGetLastPazaakResult | Global::ExecuteCommandGetLastPazaakResult | USER_DEFINED |
| 0x00797c40 | ExecuteCommandGetLastPerceived | Global::ExecuteCommandGetLastPerceived | USER_DEFINED |
| 0x00797ce0 | ExecuteCommandGetLastPerception | Global::ExecuteCommandGetLastPerception | USER_DEFINED |
| 0x00797ed0 | ExecuteCommandGetLastSpeaker | Global::ExecuteCommandGetLastSpeaker | USER_DEFINED |
| 0x00797f70 | ExecuteCommandGetLastMineDetected | Global::ExecuteCommandGetLastMineDetected | USER_DEFINED |
| 0x00797ff0 | ExecuteCommandGetLastUnlocked | Global::ExecuteCommandGetLastUnlocked | USER_DEFINED |
| 0x007980d0 | ExecuteCommandGetLastUsedBy | Global::ExecuteCommandGetLastUsedBy | USER_DEFINED |
| 0x00798180 | ExecuteCommandGetLastWeaponUsed | Global::ExecuteCommandGetLastWeaponUsed | USER_DEFINED |
| 0x00798200 | ExecuteCommandGetLeavingObject | Global::ExecuteCommandGetLeavingObject | USER_DEFINED |
| 0x00798390 | ExecuteCommandGetLoadFromSaveGame | Global::ExecuteCommandGetLoadFromSaveGame | USER_DEFINED |
| 0x007983e0 | ExecuteCommandGetLocalValue | Global::ExecuteCommandGetLocalValue | USER_DEFINED |
| 0x00798520 | ExecuteCommandGetLocation | Global::ExecuteCommandGetLocation | USER_DEFINED |
| 0x00798690 | ExecuteCommandGetLocked | Global::ExecuteCommandGetLocked | USER_DEFINED |
| 0x00798770 | ExecuteCommandGetLockInfo | Global::ExecuteCommandGetLockInfo | USER_DEFINED |
| 0x00798b40 | ExecuteCommandGetMatchedString | Global::ExecuteCommandGetMatchedString | USER_DEFINED |
| 0x00798c90 | ExecuteCommandGetMatchedStringsCount | Global::ExecuteCommandGetMatchedStringsCount | USER_DEFINED |
| 0x00798d10 | ExecuteCommandGetMatchedStringsNum | Global::ExecuteCommandGetMatchedStringsNum | USER_DEFINED |
| 0x00798d90 | ExecuteCommandGetMetaMagicFeat | Global::ExecuteCommandGetMetaMagicFeat | USER_DEFINED |
| 0x00798e20 | ExecuteCommandGetModule | Global::ExecuteCommandGetModule | USER_DEFINED |
| 0x00798ea0 | ExecuteCommandGetModuleItemStuff | Global::ExecuteCommandGetModuleItemStuff | USER_DEFINED |
| 0x00798f70 | ExecuteCommandGetModuleName | Global::ExecuteCommandGetModuleName | USER_DEFINED |
| 0x00799100 | ExecuteCommandGetMovementRate | Global::ExecuteCommandGetMovementRate | USER_DEFINED |
| 0x007991a0 | ExecuteCommandGetName | Global::ExecuteCommandGetName | USER_DEFINED |
| 0x00799310 | ExecuteCommandGetNearestObject | Global::ExecuteCommandGetNearestObject | USER_DEFINED |
| 0x0079a710 | ExecuteCommandGetNearestMine | Global::ExecuteCommandGetNearestMine | USER_DEFINED |
| 0x0079a9c0 | ExecuteCommandGetNightTrack | Global::ExecuteCommandGetNightTrack | USER_DEFINED |
| 0x0079aa50 | ExecuteCommandGetNumStackedItems | Global::ExecuteCommandGetNumStackedItems | USER_DEFINED |
| 0x0079aad0 | ExecuteCommandGetObjectByTag | Global::ExecuteCommandGetObjectByTag | USER_DEFINED |
| 0x0079ac00 | ExecuteCommandGetObjectInArea | Global::ExecuteCommandGetObjectInArea | USER_DEFINED |
| 0x0079af30 | ExecuteCommandGetObjectInShape | Global::ExecuteCommandGetObjectInShape | USER_DEFINED |
| 0x0079bd70 | ExecuteCommandGetObjectType | Global::ExecuteCommandGetObjectType | USER_DEFINED |
| 0x0079bee0 | ExecuteCommandGetObjectValid | Global::ExecuteCommandGetObjectValid | USER_DEFINED |
| 0x0079bf60 | ExecuteCommandGetObjectVisibility | Global::ExecuteCommandGetObjectVisibility | USER_DEFINED |
| 0x0079c090 | ExecuteCommandGetOwnerDemolitionsSkill | Global::ExecuteCommandGetOwnerDemolitionsSkill | USER_DEFINED |
| 0x0079c1d0 | ExecuteCommandGetLastForfeitViolation | Global::ExecuteCommandGetLastForfeitViolation | USER_DEFINED |
| 0x0079c250 | ExecuteCommandAdjustCreatureAttributes | Global::ExecuteCommandAdjustCreatureAttributes | USER_DEFINED |
| 0x0079c4b0 | ExecuteCommandAdjustCreatureSkills | Global::ExecuteCommandAdjustCreatureSkills | USER_DEFINED |
| 0x0079c5c0 | ExecuteCommandModifyBaseSavingThrow | Global::ExecuteCommandModifyBaseSavingThrow | USER_DEFINED |
| 0x0079c750 | ExecuteCommandGetObjectPersonalSpace | Global::ExecuteCommandGetObjectPersonalSpace | USER_DEFINED |
| 0x0079c800 | ExecuteCommandInfluence | Global::ExecuteCommandInfluence | USER_DEFINED |
| 0x0079cac0 | ExecuteCommandGetPartyMemberByIndex | Global::ExecuteCommandGetPartyMemberByIndex | USER_DEFINED |
| 0x0079cb60 | ExecuteCommandGetPartyMemberCount | Global::ExecuteCommandGetPartyMemberCount | USER_DEFINED |
| 0x0079cbb0 | ExecuteCommandGetPC | Global::ExecuteCommandGetPC | USER_DEFINED |
| 0x0079cc80 | ExecuteCommandGetPCLevellingUp | Global::ExecuteCommandGetPCLevellingUp | USER_DEFINED |
| 0x0079cce0 | ExecuteCommandGetPartyLeader | Global::ExecuteCommandGetPartyLeader | USER_DEFINED |
| 0x0079cd60 | ExecuteCommandGetPlaceableIllumination | Global::ExecuteCommandGetPlaceableIllumination | USER_DEFINED |
| 0x0079cde0 | ExecuteCommandGetPlotFlag | Global::ExecuteCommandGetPlotFlag | USER_DEFINED |
| 0x0079ce90 | ExecuteCommandGetPosition | Global::ExecuteCommandGetPosition | USER_DEFINED |
| 0x0079cf80 | ExecuteCommandGetRacialType | Global::ExecuteCommandGetRacialType | USER_DEFINED |
| 0x0079d050 | ExecuteCommandGetReflexAdjustedDamage | Global::ExecuteCommandGetReflexAdjustedDamage | USER_DEFINED |
| 0x0079d300 | ExecuteCommandGetReflexSavingThrow | Global::ExecuteCommandGetReflexSavingThrow | USER_DEFINED |
| 0x0079d450 | ExecuteCommandGetReputation | Global::ExecuteCommandGetReputation | USER_DEFINED |
| 0x0079d510 | ExecuteCommandGetRestrictMode | Global::ExecuteCommandGetRestrictMode | USER_DEFINED |
| 0x0079d600 | ExecuteCommandGetRunScriptVar | Global::ExecuteCommandGetRunScriptVar | USER_DEFINED |
| 0x0079d630 | ExecuteCommandGetSkillRank | Global::ExecuteCommandGetSkillRank | USER_DEFINED |
| 0x0079d740 | ExecuteCommandGetSpellCast | Global::ExecuteCommandGetSpellCast | USER_DEFINED |
| 0x0079d9b0 | ExecuteCommandGetSpellCasterItem | Global::ExecuteCommandGetSpellCasterItem | USER_DEFINED |
| 0x0079da40 | ExecuteCommandGetSpellForcePointCost | Global::ExecuteCommandGetSpellForcePointCost | USER_DEFINED |
| 0x0079db00 | ExecuteCommandGrantAbility | Global::ExecuteCommandGrantAbility | USER_DEFINED |
| 0x0079dbe0 | ExecuteCommandGetSpellFormMask | Global::ExecuteCommandGetSpellFormMask | USER_DEFINED |
| 0x0079dc70 | ExecuteCommandGetSpellBaseForcePointCost | Global::ExecuteCommandGetSpellBaseForcePointCost | USER_DEFINED |
| 0x0079dd00 | ExecuteCommandGetSpellId | Global::ExecuteCommandGetSpellId | USER_DEFINED |
| 0x0079dda0 | ExecuteCommandGetSpellSaveDC | Global::ExecuteCommandGetSpellSaveDC | USER_DEFINED |
| 0x0079df00 | ExecuteCommandGetSpellTargetLoc | Global::ExecuteCommandGetSpellTargetLoc | USER_DEFINED |
| 0x0079e050 | ExecuteCommandGetSpellTargetObject | Global::ExecuteCommandGetSpellTargetObject | USER_DEFINED |
| 0x0079e110 | ExecuteCommandGetStat | Global::ExecuteCommandGetStat | USER_DEFINED |
| 0x0079e1b0 | ExecuteCommandGetStrByStrRef | Global::ExecuteCommandGetStrByStrRef | USER_DEFINED |
| 0x0079e2c0 | ExecuteCommandGetStringLeft | Global::ExecuteCommandGetStringLeft | USER_DEFINED |
| 0x0079e430 | ExecuteCommandGetStringLength | Global::ExecuteCommandGetStringLength | USER_DEFINED |
| 0x0079e520 | ExecuteCommandGetStringLowerCase | Global::ExecuteCommandGetStringLowerCase | USER_DEFINED |
| 0x0079e650 | ExecuteCommandGetStringRight | Global::ExecuteCommandGetStringRight | USER_DEFINED |
| 0x0079e7c0 | ExecuteCommandGetStringUpperCase | Global::ExecuteCommandGetStringUpperCase | USER_DEFINED |
| 0x0079e8f0 | ExecuteCommandGetStrRefSoundDuration | Global::ExecuteCommandGetStrRefSoundDuration | USER_DEFINED |
| 0x0079ea00 | ExecuteCommandGetSubString | Global::ExecuteCommandGetSubString | USER_DEFINED |
| 0x0079ebb0 | ExecuteCommandGetTag | Global::ExecuteCommandGetTag | USER_DEFINED |
| 0x0079ed20 | ExecuteCommandGetTimeHour | Global::ExecuteCommandGetTimeHour | USER_DEFINED |
| 0x0079ed70 | ExecuteCommandGetTimeMillisecond | Global::ExecuteCommandGetTimeMillisecond | USER_DEFINED |
| 0x0079edc0 | ExecuteCommandGetTimeMinute | Global::ExecuteCommandGetTimeMinute | USER_DEFINED |
| 0x0079ee10 | ExecuteCommandGetTimeSecond | Global::ExecuteCommandGetTimeSecond | USER_DEFINED |
| 0x0079ee60 | ExecuteCommandGetTotalDamageDealt | Global::ExecuteCommandGetTotalDamageDealt | USER_DEFINED |
| 0x0079ef30 | ExecuteCommandGetMineInfo | Global::ExecuteCommandGetMineInfo | USER_DEFINED |
| 0x0079fce0 | ExecuteCommandGetUserActionsPending | Global::ExecuteCommandGetUserActionsPending | USER_DEFINED |
| 0x0079fd70 | ExecuteCommandGetUserDefinedEventNumber | Global::ExecuteCommandGetUserDefinedEventNumber | USER_DEFINED |
| 0x0079fe60 | ExecuteCommandGetWaypointByTag | Global::ExecuteCommandGetWaypointByTag | USER_DEFINED |
| 0x0079ffa0 | ExecuteCommandGetWaypointByTrigger | Global::ExecuteCommandGetWaypointByTrigger | USER_DEFINED |
| 0x007a0100 | ExecuteCommandGetWeaponRanged | Global::ExecuteCommandGetWeaponRanged | USER_DEFINED |
| 0x007a01b0 | ExecuteCommandGetWillSavingThrow | Global::ExecuteCommandGetWillSavingThrow | USER_DEFINED |
| 0x007a0300 | ExecuteCommandGetXP | Global::ExecuteCommandGetXP | USER_DEFINED |
| 0x007a03c0 | ExecuteCommandGiveGoldToCreature | Global::ExecuteCommandGiveGoldToCreature | USER_DEFINED |
| 0x007a04b0 | ExecuteCommandGiveXPToCreature | Global::ExecuteCommandGiveXPToCreature | USER_DEFINED |
| 0x007a05d0 | ExecuteCommandGiveItem | Global::ExecuteCommandGiveItem | USER_DEFINED |
| 0x007a0760 | ExecuteCommandInsertString | Global::ExecuteCommandInsertString | USER_DEFINED |
| 0x007a0930 | ExecuteCommandIntToHexString | Global::ExecuteCommandIntToHexString | USER_DEFINED |
| 0x007a0a10 | ExecuteCommandIntToString | Global::ExecuteCommandIntToString | USER_DEFINED |
| 0x007a0b00 | ExecuteCommandIsAIState | Global::ExecuteCommandIsAIState | USER_DEFINED |
| 0x007a0c40 | ExecuteCommandIsAvailableNPC | Global::ExecuteCommandIsAvailableNPC | USER_DEFINED |
| 0x007a0d10 | ExecuteCommandIsEffectValid | Global::ExecuteCommandIsEffectValid | USER_DEFINED |
| 0x007a0e60 | ExecuteCommandGetIsInConversation | Global::ExecuteCommandGetIsInConversation | USER_DEFINED |
| 0x007a0f80 | ExecuteCommandGetIsConversationActive | Global::ExecuteCommandGetIsConversationActive | USER_DEFINED |
| 0x007a0fd0 | ExecuteCommandGetCreatureMovmentType | Global::ExecuteCommandGetCreatureMovmentType | USER_DEFINED |
| 0x007a1060 | ExecuteCommandGetLastConversation | Global::ExecuteCommandGetLastConversation | USER_DEFINED |
| 0x007a10f0 | ExecuteCommandIsListening | Global::ExecuteCommandIsListening | USER_DEFINED |
| 0x007a1190 | ExecuteCommandIsNPCPartyMember | Global::ExecuteCommandIsNPCPartyMember | USER_DEFINED |
| 0x007a1230 | ExecuteCommandIsObjectPartyMember | Global::ExecuteCommandIsObjectPartyMember | USER_DEFINED |
| 0x007a12b0 | ExecuteCommandIsPC | Global::ExecuteCommandIsPC | USER_DEFINED |
| 0x007a1340 | ExecuteCommandIsStealthed | Global::ExecuteCommandIsStealthed | USER_DEFINED |
| 0x007a13c0 | ExecuteCommandIsMeditating | Global::ExecuteCommandIsMeditating | USER_DEFINED |
| 0x007a1450 | ExecuteCommandIsInTotalDefense | Global::ExecuteCommandIsInTotalDefense | USER_DEFINED |
| 0x007a14d0 | ExecuteCommandIsFormActive | Global::ExecuteCommandIsFormActive | USER_DEFINED |
| 0x007a15b0 | ExecuteCommandItemActivated | Global::ExecuteCommandItemActivated | USER_DEFINED |
| 0x007a17b0 | ExecuteCommandLocation | Global::ExecuteCommandLocation | USER_DEFINED |
| 0x007a19d0 | ExecuteCommandLocationAccess | Global::ExecuteCommandLocationAccess | USER_DEFINED |
| 0x007a1af0 | ExecuteCommandMath | Global::ExecuteCommandMath | USER_DEFINED |
| 0x007a1e10 | ExecuteCommandModuleAccess | Global::ExecuteCommandModuleAccess | USER_DEFINED |
| 0x007a2000 | ExecuteCommandMoveAwayFromObject | Global::ExecuteCommandMoveAwayFromObject | USER_DEFINED |
| 0x007a21a0 | ExecuteCommandMoveToObject | Global::ExecuteCommandMoveToObject | USER_DEFINED |
| 0x007a2530 | ExecuteCommandMoveToPoint | Global::ExecuteCommandMoveToPoint | USER_DEFINED |
| 0x007a27b0 | ExecuteCommandMusicBackground | Global::ExecuteCommandMusicBackground | USER_DEFINED |
| 0x007a2950 | ExecuteCommandMusicBattle | Global::ExecuteCommandMusicBattle | USER_DEFINED |
| 0x007a2a60 | ExecuteCommandModifyGlobal | Global::ExecuteCommandModifyGlobal | USER_DEFINED |
| 0x007a2c40 | ExecuteCommandObjectToString | Global::ExecuteCommandObjectToString | USER_DEFINED |
| 0x007a2d30 | ExecuteCommandOpenDoor | Global::ExecuteCommandOpenDoor | USER_DEFINED |
| 0x007a2e20 | ExecuteCommandOpenStore | Global::ExecuteCommandOpenStore | USER_DEFINED |
| 0x007a30e0 | ExecuteCommandPickUpItem | Global::ExecuteCommandPickUpItem | USER_DEFINED |
| 0x007a3150 | ExecuteCommandPlayAnimation | Global::ExecuteCommandPlayAnimation | USER_DEFINED |
| 0x007a3c10 | ExecuteCommandPlayRoomAnimation | Global::ExecuteCommandPlayRoomAnimation | USER_DEFINED |
| 0x007a3d70 | ExecuteCommandPlayMovie | Global::ExecuteCommandPlayMovie | USER_DEFINED |
| 0x007a3ed0 | ExecuteCommandPlayPazaak | Global::ExecuteCommandPlayPazaak | USER_DEFINED |
| 0x007a4060 | ExecuteCommandPlayRumblePattern | Global::ExecuteCommandPlayRumblePattern | USER_DEFINED |
| 0x007a40c0 | ExecuteCommandPlaySound | Global::ExecuteCommandPlaySound | USER_DEFINED |
| 0x007a4240 | ExecuteCommandPlayVisualAreaEffect | Global::ExecuteCommandPlayVisualAreaEffect | USER_DEFINED |
| 0x007a4330 | ExecuteCommandPopUpGUIPanel | Global::ExecuteCommandPopUpGUIPanel | USER_DEFINED |
| 0x007a4540 | ExecuteCommandPrintFloat | Global::ExecuteCommandPrintFloat | USER_DEFINED |
| 0x007a4710 | ExecuteCommandPrintInteger | Global::ExecuteCommandPrintInteger | USER_DEFINED |
| 0x007a4770 | ExecuteCommandPrintLogEntry | Global::ExecuteCommandPrintLogEntry | USER_DEFINED |
| 0x007a4850 | ExecuteCommandPrintObject | Global::ExecuteCommandPrintObject | USER_DEFINED |
| 0x007a48b0 | ExecuteCommandPrintString | Global::ExecuteCommandPrintString | USER_DEFINED |
| 0x007a4a20 | ExecuteCommandPrintVector | Global::ExecuteCommandPrintVector | USER_DEFINED |
| 0x007a4a90 | ExecuteCommandPlayOverlayAnimation | Global::ExecuteCommandPlayOverlayAnimation | USER_DEFINED |
| 0x007a4bb0 | ExecuteCommandPutDownItem | Global::ExecuteCommandPutDownItem | USER_DEFINED |
| 0x007a4c90 | ExecuteCommandRandomName | Global::ExecuteCommandRandomName | USER_DEFINED |
| 0x007a4dc0 | ExecuteCommandRemoveAvailableNPC | Global::ExecuteCommandRemoveAvailableNPC | USER_DEFINED |
| 0x007a4e30 | ExecuteCommandRemoveEffect | Global::ExecuteCommandRemoveEffect | USER_DEFINED |
| 0x007a5000 | ExecuteCommandRemoveEffectByID | Global::ExecuteCommandRemoveEffectByID | USER_DEFINED |
| 0x007a5170 | ExecuteCommandRemoveEffectByExactMatch | Global::ExecuteCommandRemoveEffectByExactMatch | USER_DEFINED |
| 0x007a5390 | ExecuteCommandRemoveFromParty | Global::ExecuteCommandRemoveFromParty | USER_DEFINED |
| 0x007a5410 | ExecuteCommandRemoveHeartbeat | Global::ExecuteCommandRemoveHeartbeat | USER_DEFINED |
| 0x007a5500 | ExecuteCommandRemoveJournalQuestEntry | Global::ExecuteCommandRemoveJournalQuestEntry | USER_DEFINED |
| 0x007a5650 | ExecuteCommandRemovePartyMember | Global::ExecuteCommandRemovePartyMember | USER_DEFINED |
| 0x007a5740 | ExecuteCommandResistSpell | Global::ExecuteCommandResistSpell | USER_DEFINED |
| 0x007a5df0 | ExecuteCommandRollDice | Global::ExecuteCommandRollDice | USER_DEFINED |
| 0x007a5f20 | ExecuteCommandSaveNPCState | Global::ExecuteCommandSaveNPCState | USER_DEFINED |
| 0x007a5f70 | ExecuteCommandSavingThrow | Global::ExecuteCommandSavingThrow | USER_DEFINED |
| 0x007a60e0 | ExecuteCommandSendMessageToPC | Global::ExecuteCommandSendMessageToPC | USER_DEFINED |
| 0x007a6250 | ExecuteCommandSetAreaTransitionBMP | Global::ExecuteCommandSetAreaTransitionBMP | USER_DEFINED |
| 0x007a6350 | ExecuteCommandSetAreaUnescapable | Global::ExecuteCommandSetAreaUnescapable | USER_DEFINED |
| 0x007a63c0 | ExecuteCommandSetCameraLocation | Global::ExecuteCommandSetCameraLocation | USER_DEFINED |
| 0x007a64f0 | ExecuteCommandSetCameraMode | Global::ExecuteCommandSetCameraMode | USER_DEFINED |
| 0x007a6590 | ExecuteCommandSetCommandable | Global::ExecuteCommandSetCommandable | USER_DEFINED |
| 0x007a6670 | ExecuteCommandSetCurrentForm | Global::ExecuteCommandSetCurrentForm | USER_DEFINED |
| 0x007a6730 | ExecuteCommandSetCustomToken | Global::ExecuteCommandSetCustomToken | USER_DEFINED |
| 0x007a68d0 | ExecuteCommandSetEncounterData | Global::ExecuteCommandSetEncounterData | USER_DEFINED |
| 0x007a69b0 | ExecuteCommandSetFacing | Global::ExecuteCommandSetFacing | USER_DEFINED |
| 0x007a6c60 | ExecuteCommandSetGlobalLocation | Global::ExecuteCommandSetGlobalLocation | USER_DEFINED |
| 0x007a6da0 | ExecuteCommandSetGlobalNumber | Global::ExecuteCommandSetGlobalNumber | USER_DEFINED |
| 0x007a6ee0 | ExecuteCommandSetGlobalString | Global::ExecuteCommandSetGlobalString | USER_DEFINED |
| 0x007a7000 | ExecuteCommandSetIdentified | Global::ExecuteCommandSetIdentified | USER_DEFINED |
| 0x007a7080 | ExecuteCommandSetIsDestroyable | Global::ExecuteCommandSetIsDestroyable | USER_DEFINED |
| 0x007a7280 | ExecuteCommandSetJournalQuestEntryPicture | Global::ExecuteCommandSetJournalQuestEntryPicture | USER_DEFINED |
| 0x007a7620 | ExecuteCommandSetLightsaberPowered | Global::ExecuteCommandSetLightsaberPowered | USER_DEFINED |
| 0x007a76e0 | ExecuteCommandSetListening | Global::ExecuteCommandSetListening | USER_DEFINED |
| 0x007a7780 | ExecuteCommandSetListenString | Global::ExecuteCommandSetListenString | USER_DEFINED |
| 0x007a7890 | ExecuteCommandSetLocalValue | Global::ExecuteCommandSetLocalValue | USER_DEFINED |
| 0x007a79f0 | ExecuteCommandSetLocked | Global::ExecuteCommandSetLocked | USER_DEFINED |
| 0x007a7ae0 | ExecuteCommandSetMapPinEnabled | Global::ExecuteCommandSetMapPinEnabled | USER_DEFINED |
| 0x007a7c20 | ExecuteCommandSetNPCSelectability | Global::ExecuteCommandSetNPCSelectability | USER_DEFINED |
| 0x007a7c90 | ExecuteCommandSetPlaceableIllumination | Global::ExecuteCommandSetPlaceableIllumination | USER_DEFINED |
| 0x007a7d10 | ExecuteCommandSetPlotFlag | Global::ExecuteCommandSetPlotFlag | USER_DEFINED |
| 0x007a7db0 | ExecuteCommandSetRestrictMode | Global::ExecuteCommandSetRestrictMode | USER_DEFINED |
| 0x007a7e40 | ExecuteCommandSetReturnStrref | Global::ExecuteCommandSetReturnStrref | USER_DEFINED |
| 0x007a7ee0 | ExecuteCommandSetSpeedBlurEffect | Global::ExecuteCommandSetSpeedBlurEffect | USER_DEFINED |
| 0x007a7f60 | ExecuteCommandSetSubType | Global::ExecuteCommandSetSubType | USER_DEFINED |
| 0x007a8050 | ExecuteCommandSetTime | Global::ExecuteCommandSetTime | USER_DEFINED |
| 0x007a8150 | ExecuteCommandSetForcePowerUnsuccessful | Global::ExecuteCommandSetForcePowerUnsuccessful | USER_DEFINED |
| 0x007a81d0 | ExecuteCommandSetXP | Global::ExecuteCommandSetXP | USER_DEFINED |
| 0x007a8290 | ExecuteCommandShipBuild | Global::ExecuteCommandShipBuild | USER_DEFINED |
| 0x007a82d0 | ExecuteCommandShowLevelUpGUI | Global::ExecuteCommandShowLevelUpGUI | USER_DEFINED |
| 0x007a8330 | ExecuteCommandShowPartySelection | Global::ExecuteCommandShowPartySelection | USER_DEFINED |
| 0x007a84a0 | ExecuteCommandShowUpgradeScreen | Global::ExecuteCommandShowUpgradeScreen | USER_DEFINED |
| 0x007a8670 | ExecuteCommandSignalEvent | Global::ExecuteCommandSignalEvent | USER_DEFINED |
| 0x007a8770 | ExecuteCommandSoundObjectGetFixedVariance | Global::ExecuteCommandSoundObjectGetFixedVariance | USER_DEFINED |
| 0x007a87f0 | ExecuteCommandSoundObjectGetPitchVariation | Global::ExecuteCommandSoundObjectGetPitchVariation | USER_DEFINED |
| 0x007a8870 | ExecuteCommandSoundObjectGetVolume | Global::ExecuteCommandSoundObjectGetVolume | USER_DEFINED |
| 0x007a88f0 | ExecuteCommandSoundObjectPlay | Global::ExecuteCommandSoundObjectPlay | USER_DEFINED |
| 0x007a8950 | ExecuteCommandSoundObjectSetFixedVariance | Global::ExecuteCommandSoundObjectSetFixedVariance | USER_DEFINED |
| 0x007a89d0 | ExecuteCommandSoundObjectSetPitchVariation | Global::ExecuteCommandSoundObjectSetPitchVariation | USER_DEFINED |
| 0x007a8a50 | ExecuteCommandSoundObjectSetPosition | Global::ExecuteCommandSoundObjectSetPosition | USER_DEFINED |
| 0x007a8af0 | ExecuteCommandSoundObjectSetVolume | Global::ExecuteCommandSoundObjectSetVolume | USER_DEFINED |
| 0x007a8b70 | ExecuteCommandSoundObjectStop | Global::ExecuteCommandSoundObjectStop | USER_DEFINED |
| 0x007a8bd0 | ExecuteCommandSoundObjectFadeAndStop | Global::ExecuteCommandSoundObjectFadeAndStop | USER_DEFINED |
| 0x007a8c70 | ExecuteCommandSpawnAvailableNPC | Global::ExecuteCommandSpawnAvailableNPC | USER_DEFINED |
| 0x007a8d60 | ExecuteCommandSpeakOneLinerConversation | Global::ExecuteCommandSpeakOneLinerConversation | USER_DEFINED |
| 0x007a8eb0 | ExecuteCommandSpeakString | Global::ExecuteCommandSpeakString | USER_DEFINED |
| 0x007a9110 | ExecuteCommandSpeakStrRef | Global::ExecuteCommandSpeakStrRef | USER_DEFINED |
| 0x007a9260 | ExecuteCommandStartNewModule | Global::ExecuteCommandStartNewModule | USER_DEFINED |
| 0x007a97c0 | ExecuteCommandStopRumblePattern | Global::ExecuteCommandStopRumblePattern | USER_DEFINED |
| 0x007a9820 | ExecuteCommandStringConversions | Global::ExecuteCommandStringConversions | USER_DEFINED |
| 0x007a9940 | ExecuteCommandSurrenderToEnemies | Global::ExecuteCommandSurrenderToEnemies | USER_DEFINED |
| 0x007a9a50 | ExecuteCommandSwitchPlayerCreature | Global::ExecuteCommandSwitchPlayerCreature | USER_DEFINED |
| 0x007a9ac0 | ExecuteCommandSpawnMine | Global::ExecuteCommandSpawnMine | USER_DEFINED |
| 0x007a9fd0 | ExecuteCommandSetForfeitConditions | Global::ExecuteCommandSetForfeitConditions | USER_DEFINED |
| 0x007aa040 | ExecuteCommandTakeGoldFromCreature | Global::ExecuteCommandTakeGoldFromCreature | USER_DEFINED |
| 0x007aa460 | ExecuteCommandTalent | Global::ExecuteCommandTalent | USER_DEFINED |
| 0x007aa620 | ExecuteCommandTalentAccess | Global::ExecuteCommandTalentAccess | USER_DEFINED |
| 0x007aa790 | ExecuteCommandTimeConversions | Global::ExecuteCommandTimeConversions | USER_DEFINED |
| 0x007aa860 | ExecuteCommandTestString | Global::ExecuteCommandTestString | USER_DEFINED |
| 0x007aa9c0 | ExecuteCommandUnequipItem | Global::ExecuteCommandUnequipItem | USER_DEFINED |
| 0x007aaac0 | ExecuteCommandVector | Global::ExecuteCommandVector | USER_DEFINED |
| 0x007aab50 | ExecuteCommandVectorConversions | Global::ExecuteCommandVectorConversions | USER_DEFINED |
| 0x007aacd0 | ExecuteCommandVectorMagnitude | Global::ExecuteCommandVectorMagnitude | USER_DEFINED |
| 0x007aad40 | ExecuteCommandVectorNormalize | Global::ExecuteCommandVectorNormalize | USER_DEFINED |
| 0x007aae00 | ExecuteCommandVersusEffect | Global::ExecuteCommandVersusEffect | USER_DEFINED |
| 0x007ab560 | ExecuteCommandWait | Global::ExecuteCommandWait | USER_DEFINED |
| 0x007ab660 | ExecuteCommandGetIsLinkImmune | Global::ExecuteCommandGetIsLinkImmune | USER_DEFINED |
| 0x007ab790 | ExecuteCommandGetPartyAIStyle | Global::ExecuteCommandGetPartyAIStyle | USER_DEFINED |
| 0x007ab7e0 | ExecuteCommandGetNPCAIStyle | Global::ExecuteCommandGetNPCAIStyle | USER_DEFINED |
| 0x007ab8a0 | ExecuteCommandSetPartyLeader | Global::ExecuteCommandSetPartyLeader | USER_DEFINED |
| 0x007abb70 | ExecuteCommandSetPartyAIStyle | Global::ExecuteCommandSetPartyAIStyle | USER_DEFINED |
| 0x007abbd0 | ExecuteCommandSetNPCAIStyle | Global::ExecuteCommandSetNPCAIStyle | USER_DEFINED |
| 0x007abca0 | ExecuteCommandClearAllEffects | Global::ExecuteCommandClearAllEffects | USER_DEFINED |
| 0x007abd00 | ExecuteCommandGetStandardFaction | Global::ExecuteCommandGetStandardFaction | USER_DEFINED |
| 0x007abdf0 | ExecuteCommandGivePlotXP | Global::ExecuteCommandGivePlotXP | USER_DEFINED |
| 0x007abee0 | ExecuteCommandGetMinOneHP | Global::ExecuteCommandGetMinOneHP | USER_DEFINED |
| 0x007abf80 | ExecuteCommandSetMinOneHP | Global::ExecuteCommandSetMinOneHP | USER_DEFINED |
| 0x007ac030 | ExecuteCommandSetFadeIn | Global::ExecuteCommandSetFadeIn | USER_DEFINED |
| 0x007ac150 | ExecuteCommandSetFadeOut | Global::ExecuteCommandSetFadeOut | USER_DEFINED |
| 0x007ac240 | ExecuteCommandGetAttacker | Global::ExecuteCommandGetAttacker | USER_DEFINED |
| 0x007ac310 | ExecuteCommandSetFormation | Global::ExecuteCommandSetFormation | USER_DEFINED |
| 0x007ac3d0 | ExecuteCommandGetIsDebilitated | Global::ExecuteCommandGetIsDebilitated | USER_DEFINED |
| 0x007ac460 | ExecuteCommandChangeByFaction | Global::ExecuteCommandChangeByFaction | USER_DEFINED |
| 0x007ac670 | ExecuteCommandGetSubScreenID | Global::ExecuteCommandGetSubScreenID | USER_DEFINED |
| 0x007ac6e0 | ExecuteCommandCancelCombat | Global::ExecuteCommandCancelCombat | USER_DEFINED |
| 0x007ac770 | ExecuteCommandPauseGame | Global::ExecuteCommandPauseGame | USER_DEFINED |
| 0x007ac7c0 | ExecuteCommandGetItemStackSize | Global::ExecuteCommandGetItemStackSize | USER_DEFINED |
| 0x007ac840 | ExecuteCommandSetItemStackSize | Global::ExecuteCommandSetItemStackSize | USER_DEFINED |
| 0x007acab0 | ExecuteCommandShowGalaxyMapGui | Global::ExecuteCommandShowGalaxyMapGui | USER_DEFINED |
| 0x007acb00 | ExecuteCommandSetGalaxyMapPlanetAvailable | Global::ExecuteCommandSetGalaxyMapPlanetAvailable | USER_DEFINED |
| 0x007acb70 | ExecuteCommandGetGalaxyMapPlanetAvailable | Global::ExecuteCommandGetGalaxyMapPlanetAvailable | USER_DEFINED |
| 0x007acbe0 | ExecuteCommandSetGalaxyMapPlanetSelectable | Global::ExecuteCommandSetGalaxyMapPlanetSelectable | USER_DEFINED |
| 0x007acc50 | ExecuteCommandGetGalaxyMapPlanetSelectable | Global::ExecuteCommandGetGalaxyMapPlanetSelectable | USER_DEFINED |
| 0x007accc0 | ExecuteCommandGetGalaxyMapSelectedPlanet | Global::ExecuteCommandGetGalaxyMapSelectedPlanet | USER_DEFINED |
| 0x007acd10 | ExecuteCommandSetItemNonEquippable | Global::ExecuteCommandSetItemNonEquippable | USER_DEFINED |
| 0x007acda0 | ExecuteCommandGetButtonMashCheck | Global::ExecuteCommandGetButtonMashCheck | USER_DEFINED |
| 0x007acdf0 | ExecuteCommandSetButtonMashCheck | Global::ExecuteCommandSetButtonMashCheck | USER_DEFINED |
| 0x007ace40 | ExecuteCommandSetDialogPlaceableCamera | Global::ExecuteCommandSetDialogPlaceableCamera | USER_DEFINED |
| 0x007acec0 | ExecuteCommandGetSoloMode | Global::ExecuteCommandGetSoloMode | USER_DEFINED |
| 0x007acf10 | ExecuteCommandGetMaxStealthXP | Global::ExecuteCommandGetMaxStealthXP | USER_DEFINED |
| 0x007acf90 | ExecuteCommandSetMaxStealthXP | Global::ExecuteCommandSetMaxStealthXP | USER_DEFINED |
| 0x007ad000 | ExecuteCommandGetCurrentStealthXP | Global::ExecuteCommandGetCurrentStealthXP | USER_DEFINED |
| 0x007ad080 | ExecuteCommandSetCurrentStealthXP | Global::ExecuteCommandSetCurrentStealthXP | USER_DEFINED |
| 0x007ad0f0 | ExecuteCommandAwardStealthXP | Global::ExecuteCommandAwardStealthXP | USER_DEFINED |
| 0x007ad170 | ExecuteCommandGetStealthXPEnabled | Global::ExecuteCommandGetStealthXPEnabled | USER_DEFINED |
| 0x007ad1f0 | ExecuteCommandSetStealthXPEnabled | Global::ExecuteCommandSetStealthXPEnabled | USER_DEFINED |
| 0x007ad270 | ExecuteCommandGetSubRace | Global::ExecuteCommandGetSubRace | USER_DEFINED |
| 0x007ad310 | ExecuteCommandGetBlockingCreature | Global::ExecuteCommandGetBlockingCreature | USER_DEFINED |
| 0x007ad3a0 | ExecuteCommandGetFoundEnemyCreature | Global::ExecuteCommandGetFoundEnemyCreature | USER_DEFINED |
| 0x007ad430 | ExecuteCommandGetStealthXPDecrement | Global::ExecuteCommandGetStealthXPDecrement | USER_DEFINED |
| 0x007ad4b0 | ExecuteCommandSetStealthXPDecrement | Global::ExecuteCommandSetStealthXPDecrement | USER_DEFINED |
| 0x007ad530 | ExecuteCommandDuplicateHeadAppearance | Global::ExecuteCommandDuplicateHeadAppearance | USER_DEFINED |
| 0x007ad6c0 | ExecuteCommandSetLockOrientationInDialog | Global::ExecuteCommandSetLockOrientationInDialog | USER_DEFINED |
| 0x007ad750 | ExecuteCommandSetLockHeadFollowInDialog | Global::ExecuteCommandSetLockHeadFollowInDialog | USER_DEFINED |
| 0x007ad7e0 | ExecuteCommandEnableRendering | Global::ExecuteCommandEnableRendering | USER_DEFINED |
| 0x007add30 | ExecuteCommandEnableVideoEffect | Global::ExecuteCommandEnableVideoEffect | USER_DEFINED |
| 0x007adda0 | ExecuteCommandDisableVideoEffect | Global::ExecuteCommandDisableVideoEffect | USER_DEFINED |
| 0x007addf0 | ExecuteCommandRevealMap | Global::ExecuteCommandRevealMap | USER_DEFINED |
| 0x007adec0 | ExecuteCommandSetTutorialWindowsEnabled | Global::ExecuteCommandSetTutorialWindowsEnabled | USER_DEFINED |
| 0x007adf30 | ExecuteCommandShowTutorialWindow | Global::ExecuteCommandShowTutorialWindow | USER_DEFINED |
| 0x007adf80 | ExecuteCommandIsCreditSequenceInProgress | Global::ExecuteCommandIsCreditSequenceInProgress | USER_DEFINED |
| 0x007adfd0 | ExecuteCommandStartCreditSequence | Global::ExecuteCommandStartCreditSequence | USER_DEFINED |
| 0x007ae110 | ExecuteCommandGetDifficultyMultiplier | Global::ExecuteCommandGetDifficultyMultiplier | USER_DEFINED |
| 0x007ae1f0 | ExecuteCommandSetEffectIcon | Global::ExecuteCommandSetEffectIcon | USER_DEFINED |
| 0x007ae4d0 | ExecuteCommandFaceObjectAwayFromObject | Global::ExecuteCommandFaceObjectAwayFromObject | USER_DEFINED |
| 0x007ae650 | ExecuteCommandSetAreaFogColor | Global::ExecuteCommandSetAreaFogColor | USER_DEFINED |
| 0x007ae730 | ExecuteCommandChangeItemCost | Global::ExecuteCommandChangeItemCost | USER_DEFINED |
| 0x007ae890 | ExecuteCommandGetIsLiveContentAvailable | Global::ExecuteCommandGetIsLiveContentAvailable | USER_DEFINED |
| 0x007ae9d0 | ExecuteCommandResetDialogState | Global::ExecuteCommandResetDialogState | USER_DEFINED |
| 0x007aea00 | ExecuteCommandSetGoodEvilValue | Global::ExecuteCommandSetGoodEvilValue | USER_DEFINED |
| 0x007aeae0 | ExecuteCommandGetIsPoisoned | Global::ExecuteCommandGetIsPoisoned | USER_DEFINED |
| 0x007aeba0 | ExecuteCommandSetSoloMode | Global::ExecuteCommandSetSoloMode | USER_DEFINED |
| 0x007aebf0 | ExecuteCommandCancelPostDialogCharacterSwitch | Global::ExecuteCommandCancelPostDialogCharacterSwitch | USER_DEFINED |
| 0x007aec30 | ExecuteCommandSetMaxHitPoints | Global::ExecuteCommandSetMaxHitPoints | USER_DEFINED |
| 0x007aece0 | ExecuteCommandNoClicksFor | Global::ExecuteCommandNoClicksFor | USER_DEFINED |
| 0x007aed70 | ExecuteCommandHoldWorldFadeInForDialog | Global::ExecuteCommandHoldWorldFadeInForDialog | USER_DEFINED |
| 0x007aedb0 | ExecuteCommandSuppressStatusSummaryEntry | Global::ExecuteCommandSuppressStatusSummaryEntry | USER_DEFINED |
| 0x007aee00 | ExecuteCommandGetCheatCode | Global::ExecuteCommandGetCheatCode | USER_DEFINED |
| 0x007aee50 | ExecuteCommandSetMusicVolume | Global::ExecuteCommandSetMusicVolume | USER_DEFINED |
| 0x007aeef0 | ExecuteCommandCreateItemOnFloor | Global::ExecuteCommandCreateItemOnFloor | USER_DEFINED |
| 0x007af280 | ExecuteCommandSetAvailableNPCID | Global::ExecuteCommandSetAvailableNPCID | USER_DEFINED |
| 0x007af2f0 | ExecuteCommandGetScriptParameter | Global::ExecuteCommandGetScriptParameter | USER_DEFINED |
| 0x007af360 | ExecuteCommandGetScriptStringParameter | Global::ExecuteCommandGetScriptStringParameter | USER_DEFINED |
| 0x007af400 | ExecuteCommandSetFadeUntilScript | Global::ExecuteCommandSetFadeUntilScript | USER_DEFINED |
| 0x007af440 | ExecuteCommandShowChemicalUpgradeScreen | Global::ExecuteCommandShowChemicalUpgradeScreen | USER_DEFINED |
| 0x007af4a0 | ExecuteCommandShowSwoopUpgradeScreen | Global::ExecuteCommandShowSwoopUpgradeScreen | USER_DEFINED |
| 0x007af4d0 | ExecuteCommandSetFakeCombatState | Global::ExecuteCommandSetFakeCombatState | USER_DEFINED |
| 0x007af5b0 | ExecuteCommandSetOrientOnClick | Global::ExecuteCommandSetOrientOnClick | USER_DEFINED |
| 0x007af640 | ExecuteCommandGetRacialSubType | Global::ExecuteCommandGetRacialSubType | USER_DEFINED |
| 0x007af710 | ExecuteCommandSetBonusForcePoints | Global::ExecuteCommandSetBonusForcePoints | USER_DEFINED |
| 0x007af840 | ExecuteCommandAddBonusForcePoints | Global::ExecuteCommandAddBonusForcePoints | USER_DEFINED |
| 0x007af950 | ExecuteCommandGetBonusForcePoints | Global::ExecuteCommandGetBonusForcePoints | USER_DEFINED |
| 0x007af9f0 | ExecuteCommandIsMoviePlaying | Global::ExecuteCommandIsMoviePlaying | USER_DEFINED |
| 0x007afa40 | ExecuteCommandQueueMovie | Global::ExecuteCommandQueueMovie | USER_DEFINED |
| 0x007afb20 | ExecuteCommandPlayMovieQueue | Global::ExecuteCommandPlayMovieQueue | USER_DEFINED |
| 0x007afb70 | ExecuteCommandYavinHackCloseDoor | Global::ExecuteCommandYavinHackCloseDoor | USER_DEFINED |
| 0x007afbd0 | ExecuteCommandSetHealTarget | Global::ExecuteCommandSetHealTarget | USER_DEFINED |
| 0x007afc50 | ExecuteCommandGetHealTarget | Global::ExecuteCommandGetHealTarget | USER_DEFINED |
| 0x007afcd0 | ExecuteCommandGetRandomDestination | Global::ExecuteCommandGetRandomDestination | USER_DEFINED |
| 0x007b0250 | ExecuteCommandSetKeepStealthInDialog | Global::ExecuteCommandSetKeepStealthInDialog | USER_DEFINED |
| 0x007b02b0 | ExecuteCommandHasLineOfSight | Global::ExecuteCommandHasLineOfSight | USER_DEFINED |
| 0x007b0510 | ExecuteCommandShowDemoScreen | Global::ExecuteCommandShowDemoScreen | USER_DEFINED |
| 0x007b06b0 | ExecuteCommandIsRunning | Global::ExecuteCommandIsRunning | USER_DEFINED |
| 0x007b0750 | ExecuteCommandAddAvailablePUPByTemplate | Global::ExecuteCommandAddAvailablePUPByTemplate | USER_DEFINED |
| 0x007b0870 | ExecuteCommandAddAvailablePUPByObject | Global::ExecuteCommandAddAvailablePUPByObject | USER_DEFINED |
| 0x007b0910 | ExecuteCommandAssignPUP | Global::ExecuteCommandAssignPUP | USER_DEFINED |
| 0x007b09a0 | ExecuteCommandSpawnAvailablePUP | Global::ExecuteCommandSpawnAvailablePUP | USER_DEFINED |
| 0x007b0a90 | ExecuteCommandAddPartyPuppet | Global::ExecuteCommandAddPartyPuppet | USER_DEFINED |
| 0x007b0b80 | ExecuteCommandGetPUPOwner | Global::ExecuteCommandGetPUPOwner | USER_DEFINED |
| 0x007b0c30 | ExecuteCommandGetIsPuppet | Global::ExecuteCommandGetIsPuppet | USER_DEFINED |
| 0x007b0cb0 | ExecuteCommandActionFollowOwner | Global::ExecuteCommandActionFollowOwner | USER_DEFINED |
| 0x007b0d70 | ExecuteCommandGetIsPartyLeader | Global::ExecuteCommandGetIsPartyLeader | USER_DEFINED |
| 0x007b0e10 | ExecuteCommandRemoveNPCFromPartyToBase | Global::ExecuteCommandRemoveNPCFromPartyToBase | USER_DEFINED |
| 0x007b0f10 | ExecuteCommandGetIsXBox | Global::ExecuteCommandGetIsXBox | USER_DEFINED |
| 0x007b0f50 | ExecuteCommandActionSwitchWeapons | Global::ExecuteCommandActionSwitchWeapons | USER_DEFINED |
| 0x007b0fd0 | ExecuteCommandUnlockAllMusic | Global::ExecuteCommandUnlockAllMusic | USER_DEFINED |
| 0x007b10d0 | ExecuteCommandDisableMap | Global::ExecuteCommandDisableMap | USER_DEFINED |
| 0x007b1130 | ExecuteCommandDetonateMine | Global::ExecuteCommandDetonateMine | USER_DEFINED |
| 0x007b11e0 | ExecuteCommandDisableHealthRegen | Global::ExecuteCommandDisableHealthRegen | USER_DEFINED |
| 0x007b1240 | ExecuteCommandSetDisableTransit | Global::ExecuteCommandSetDisableTransit | USER_DEFINED |
| 0x007b12c0 | ExecuteCommandSetInputClass | Global::ExecuteCommandSetInputClass | USER_DEFINED |
| 0x007b1320 | ExecuteCommandForceAlwaysUpdate | Global::ExecuteCommandForceAlwaysUpdate | USER_DEFINED |
| 0x007b13b0 | ExecuteCommandEnableRain | Global::ExecuteCommandEnableRain | USER_DEFINED |
| 0x007b1470 | ExecuteCommandDisplayMessageBox | Global::ExecuteCommandDisplayMessageBox | USER_DEFINED |
| 0x007b15f0 | ExecuteCommandDisplayDatapad | Global::ExecuteCommandDisplayDatapad | USER_DEFINED |
| 0x007b17d0 | ExecuteCommandGetCombatActionPending | Global::ExecuteCommandGetCombatActionPending | USER_DEFINED |
| 0x007b1890 | ExecuteCommandSaveNPCByObject | Global::ExecuteCommandSaveNPCByObject | USER_DEFINED |
| 0x007b1910 | ExecuteCommandSavePUPByObject | Global::ExecuteCommandSavePUPByObject | USER_DEFINED |
| 0x007b1990 | ExecuteCommandGetIsPlayerMadeCharacter | Global::ExecuteCommandGetIsPlayerMadeCharacter | USER_DEFINED |
| 0x007b1a30 | ExecuteCommandRebuildPartyTable | Global::ExecuteCommandRebuildPartyTable | USER_DEFINED |
| 0x007bcce0 | UpdateLoadScreen | Global::UpdateLoadScreen | USER_DEFINED |
| 0x007c02e0 | CExoLinkedList<CSWSPlayerLUOInventoryItem>::AddHead | CExoLinkedList_CSWSPlayerLUOInventoryItem_::AddHead | USER_DEFINED |
| 0x007c0300 | CExoLinkedList<CAurObject>::RemoveHead | CExoLinkedList_CAurObject_::RemoveHead | USER_DEFINED |
| 0x007c0490 | CSWSStore::~CSWSStore | CSWSStore::Destructor_2 | USER_DEFINED |
| 0x007c17d0 | CSWSItemPropertyHandler::~CSWSItemPropertyHandler | CSWSItemPropertyHandler::Destructor_2 | USER_DEFINED |
| 0x007d35e0 | CSWSMessage::SendServerToPlayerGameObjUpdate | CSWSMessage::SendServerToPlayerGameObjUpdate_2 | USER_DEFINED |
| 0x007de2a0 | CSWSMessage::ComputeInventoryUpdateRequired | CSWSMessage::ComputeInventoryUpdateRequired_2 | USER_DEFINED |
| 0x007e4350 | CExoArrayList<CAurObject*>::SetSize | CExoArrayList_CAurObject__::SetSize | USER_DEFINED |
| 0x007e4430 | CExoArrayList<_FILETIME>::Add | CExoArrayList__FILETIME_::Add | USER_DEFINED |
| 0x007e44a0 | CExoArrayList<int>::AddUnique | CExoArrayList_int_::AddUnique | USER_DEFINED |
| 0x007e4720 | CSWSFaction::CSWSFaction | CSWSFaction::Constructor_2 | USER_DEFINED |
| 0x007e63c0 | CSWSAreaOfEffectObject::~CSWSAreaOfEffectObject | CSWSAreaOfEffectObject::Destructor_2 | USER_DEFINED |
| 0x007e9520 | CSWSEncounter::~CSWSEncounter | CSWSEncounter::Destructor_2 | USER_DEFINED |
| 0x007f0e50 | CExoArrayList<CSWSScriptVar>::~CExoArrayList | CExoArrayList_CSWSScriptVar_::Destructor | USER_DEFINED |
| 0x007f0e90 | CExoArrayList<CSWSScriptVar>::Add | CExoArrayList_CSWSScriptVar_::Add | USER_DEFINED |
| 0x007f0f60 | CExoArrayList<CSWSScriptVar>::DelIndex | CExoArrayList_CSWSScriptVar_::DelIndex | USER_DEFINED |
| 0x007f3780 | CSWSPlayerLastUpdateObject::ClearVisibilityList | CSWSPlayerLastUpdateObject::ClearVisibilityList_2 | USER_DEFINED |
| 0x007f3890 | CExoLinkedList<CSWSPlayerLUOInventoryItem>::GetPrev | CExoLinkedList_CSWSPlayerLUOInventoryItem_::GetPrev | USER_DEFINED |
| 0x007f3ac0 | GetSkillRankChange | Global::GetSkillRankChange | USER_DEFINED |
| 0x007f5850 | CSWSWaypoint::~CSWSWaypoint | CSWSWaypoint::Destructor_2 | USER_DEFINED |
| 0x007f6340 | CSWSSoundObject::~CSWSSoundObject | CSWSSoundObject::Destructor_2 | USER_DEFINED |
| 0x007f7980 | CResMDL::~CResMDL | CResMDL::Destructor_2 | USER_DEFINED |
| 0x007f7a30 | CResBWM::~CResBWM | CResBWM::Destructor_2 | USER_DEFINED |
| 0x007f88d0 | CSWPlaceableSurfaceMesh | Global::CSWPlaceableSurfaceMesh | USER_DEFINED |
| 0x007f97c0 | ExecuteCommandAurPostString | Global::ExecuteCommandAurPostString | USER_DEFINED |
| 0x007f9b00 | ExecuteCommandSWMG_GetObjectByName | Global::ExecuteCommandSWMG_GetObjectByName | USER_DEFINED |
| 0x007f9c00 | ExecuteCommandSWMG_PlayAnimation | Global::ExecuteCommandSWMG_PlayAnimation | USER_DEFINED |
| 0x007f9e00 | ExecuteCommandSWMG_GetLastBulletInfo | Global::ExecuteCommandSWMG_GetLastBulletInfo | USER_DEFINED |
| 0x007f9f30 | ExecuteCommandSWMG_GetLastBulletShooter | Global::ExecuteCommandSWMG_GetLastBulletShooter | USER_DEFINED |
| 0x007fa060 | ExecuteCommandSWMG_DefaultImplementations | Global::ExecuteCommandSWMG_DefaultImplementations | USER_DEFINED |
| 0x007fa180 | ExecuteCommandSWMG_GetLastObjectHit | Global::ExecuteCommandSWMG_GetLastObjectHit | USER_DEFINED |
| 0x007fa250 | ExecuteCommandSWMG_GetObjectName | Global::ExecuteCommandSWMG_GetObjectName | USER_DEFINED |
| 0x007fa370 | ExecuteCommandSWMG_IsSomething | Global::ExecuteCommandSWMG_IsSomething | USER_DEFINED |
| 0x007fa4d0 | ExecuteCommandSWMG_GetLastHPChange | Global::ExecuteCommandSWMG_GetLastHPChange | USER_DEFINED |
| 0x007fa540 | ExecuteCommandSWMG_RemoveAnimation | Global::ExecuteCommandSWMG_RemoveAnimation | USER_DEFINED |
| 0x007fa650 | ExecuteCommandSWMG_GetCameraClipping | Global::ExecuteCommandSWMG_GetCameraClipping | USER_DEFINED |
| 0x007fa6e0 | ExecuteCommandSWMG_SetCameraClipping | Global::ExecuteCommandSWMG_SetCameraClipping | USER_DEFINED |
| 0x007fa760 | ExecuteCommandSWMG_GetPlayer | Global::ExecuteCommandSWMG_GetPlayer | USER_DEFINED |
| 0x007fa7d0 | ExecuteCommandSWMG_GetMiniGameCounts | Global::ExecuteCommandSWMG_GetMiniGameCounts | USER_DEFINED |
| 0x007fa860 | ExecuteCommandSWMG_GetMiniGameObjects | Global::ExecuteCommandSWMG_GetMiniGameObjects | USER_DEFINED |
| 0x007fa920 | ExecuteCommandSWMG_GetFollowerIntInfo | Global::ExecuteCommandSWMG_GetFollowerIntInfo | USER_DEFINED |
| 0x007faa80 | ExecuteCommandSWMG_StartInvulnerability | Global::ExecuteCommandSWMG_StartInvulnerability | USER_DEFINED |
| 0x007faaf0 | ExecuteCommandSWMG_GetFollowerFloatInfo | Global::ExecuteCommandSWMG_GetFollowerFloatInfo | USER_DEFINED |
| 0x007faba0 | ExecuteCommandSWMG_GetFollowerPosition | Global::ExecuteCommandSWMG_GetFollowerPosition | USER_DEFINED |
| 0x007fad60 | ExecuteCommandSWMG_SetFollowerFloatInfo | Global::ExecuteCommandSWMG_SetFollowerFloatInfo | USER_DEFINED |
| 0x007fae10 | ExecuteCommandSWMG_GetGunBankStringInfo | Global::ExecuteCommandSWMG_GetGunBankStringInfo | USER_DEFINED |
| 0x007faf80 | ExecuteCommandSWMG_GetGunBankIntInfo | Global::ExecuteCommandSWMG_GetGunBankIntInfo | USER_DEFINED |
| 0x007fb080 | ExecuteCommandSWMG_GetGunBankFloatInfo | Global::ExecuteCommandSWMG_GetGunBankFloatInfo | USER_DEFINED |
| 0x007fb170 | ExecuteCommandSWMG_GetTargettingGunBankFloatInfo | Global::ExecuteCommandSWMG_GetTargettingGunBankFloatInfo | USER_DEFINED |
| 0x007fb290 | ExecuteCommandSWMG_SetGunBankStringInfo | Global::ExecuteCommandSWMG_SetGunBankStringInfo | USER_DEFINED |
| 0x007fb420 | ExecuteCommandSWMG_SetGunBankIntInfo | Global::ExecuteCommandSWMG_SetGunBankIntInfo | USER_DEFINED |
| 0x007fb500 | ExecuteCommandSWMG_SetGunBankFloatInfo | Global::ExecuteCommandSWMG_SetGunBankFloatInfo | USER_DEFINED |
| 0x007fb600 | ExecuteCommandSWMG_SetTargettingGunBankFloatInfo | Global::ExecuteCommandSWMG_SetTargettingGunBankFloatInfo | USER_DEFINED |
| 0x007fb740 | ExecuteCommandSWMG_GetLastBulletHitPart | Global::ExecuteCommandSWMG_GetLastBulletHitPart | USER_DEFINED |
| 0x007fb810 | ExecuteCommandSWMG_GetPlayerFloatInfo | Global::ExecuteCommandSWMG_GetPlayerFloatInfo | USER_DEFINED |
| 0x007fb900 | ExecuteCommandSWMG_GetPlayerVectorInfo | Global::ExecuteCommandSWMG_GetPlayerVectorInfo | USER_DEFINED |
| 0x007fbaa0 | ExecuteCommandSWMG_SetPlayerFloatInfo | Global::ExecuteCommandSWMG_SetPlayerFloatInfo | USER_DEFINED |
| 0x007fbb90 | ExecuteCommandSWMG_SetPlayerVectorInfo | Global::ExecuteCommandSWMG_SetPlayerVectorInfo | USER_DEFINED |
| 0x007fbd00 | ExecuteCommandSWMG_GetSoundProperty | Global::ExecuteCommandSWMG_GetSoundProperty | USER_DEFINED |
| 0x007fbdf0 | ExecuteCommandSWMG_SetSoundProperty | Global::ExecuteCommandSWMG_SetSoundProperty | USER_DEFINED |
| 0x007fbee0 | ExecuteCommandSWMG_SetLateralAccelPerSec | Global::ExecuteCommandSWMG_SetLateralAccelPerSec | USER_DEFINED |
| 0x007fbf80 | ExecuteCommandSWMG_GetLateralAccelPerSec | Global::ExecuteCommandSWMG_GetLateralAccelPerSec | USER_DEFINED |
| 0x007fbfd0 | ExecuteCommandSWMG_GetSwoopUpgrade | Global::ExecuteCommandSWMG_GetSwoopUpgrade | USER_DEFINED |
| 0x007fc050 | ExecuteCommandSWMG_GetTrackPosition | Global::ExecuteCommandSWMG_GetTrackPosition | USER_DEFINED |
| 0x007fc120 | ExecuteCommandSWMG_SetFollowerPosition | Global::ExecuteCommandSWMG_SetFollowerPosition | USER_DEFINED |
| 0x007fc190 | ExecuteCommandSWMG_DestroyMiniGameObject | Global::ExecuteCommandSWMG_DestroyMiniGameObject | USER_DEFINED |
| 0x007fc210 | ExecuteCommandSWMG_SetJumpSpeed | Global::ExecuteCommandSWMG_SetJumpSpeed | USER_DEFINED |
| 0x007fc260 | ExecuteCommandSWMG_PlayerApplyForce | Global::ExecuteCommandSWMG_PlayerApplyForce | USER_DEFINED |
| 0x007fc570 | CSWSMessage::~CSWSMessage | CSWSMessage::Destructor_2 | USER_DEFINED |
| 0x00809db0 | GetTokenPair | Global::GetTokenPair | USER_DEFINED |
| 0x0080abc0 | CSWSSysAdmin::~CSWSSysAdmin | CSWSSysAdmin::Destructor_2 | USER_DEFINED |
| 0x0080ae50 | ConvertToCalendarDay | Global::ConvertToCalendarDay | USER_DEFINED |
| 0x0080aed0 | ConvertToTimeOfDay | Global::ConvertToTimeOfDay | USER_DEFINED |
| 0x0080af80 | ConvertFromCalendarDay | Global::ConvertFromCalendarDay | USER_DEFINED |
| 0x0080b0b0 | SetWorldTime__ulong_ulong_ulong_ulong_ulong_ulong_ulong_int | Global::SetWorldTime__ulong_ulong_ulong_ulong_ulong_ulong_ulong_int | USER_DEFINED |
| 0x0080b5c0 | GetWorldTimeTimeOfDay | Global::GetWorldTimeTimeOfDay | USER_DEFINED |
| 0x0080b720 | SetSnapshotScale | Global::SetSnapshotScale | USER_DEFINED |
| 0x0080b750 | GetCalendarDayFromSeconds | Global::GetCalendarDayFromSeconds | USER_DEFINED |
| 0x0080b7e0 | GetTimeOfDayFromSeconds | Global::GetTimeOfDayFromSeconds | USER_DEFINED |
| 0x0080ba70 | DegToRad | Global::DegToRad | USER_DEFINED |
| 0x0080ba90 | LookupCosine32Radians | Global::LookupCosine32Radians | USER_DEFINED |
| 0x0080bad0 | LookupSine32Radians | Global::LookupSine32Radians | USER_DEFINED |
| 0x0080bb10 | Round | Global::Round | USER_DEFINED |
| 0x0080bb50 | VectorMin | Global::VectorMin | USER_DEFINED |
| 0x0080bbd0 | VectorMax | Global::VectorMax | USER_DEFINED |
| 0x0080bc50 | NormalOf | Global::NormalOf | USER_DEFINED |
| 0x0080c0a0 | float_magnitude_Vector_const_ref | Global::float_magnitude_Vector_const_ref | USER_DEFINED |
| 0x0080c170 | normalize | Global::normalize | USER_DEFINED |
| 0x0080c200 | planelineintersection | Global::planelineintersection | USER_DEFINED |
| 0x0080c380 | PlaneHeightAtPoint | Global::PlaneHeightAtPoint | USER_DEFINED |
| 0x0080c3d0 | lineproject | Global::lineproject | USER_DEFINED |
| 0x0080c590 | segmentproject | Global::segmentproject | USER_DEFINED |
| 0x0080c8b0 | Transpose | Global::Transpose | USER_DEFINED |
| 0x0080ca20 | operator* | Global::operator* | USER_DEFINED |
| 0x0080d120 | normalize | Global::normalize_2 | USER_DEFINED |
| 0x0080d220 | slerp | Global::slerp | USER_DEFINED |
| 0x0080d4b0 | YawPitchRoll | Global::YawPitchRoll | USER_DEFINED |
| 0x0080d740 | Pitch | Global::Pitch | USER_DEFINED |
| 0x0080d820 | Yaw | Global::Yaw | USER_DEFINED |
| 0x0080d900 | float_Roll_Quaternion | Global::float_Roll_Quaternion | USER_DEFINED |
| 0x0080dbe0 | Yaw | Global::Yaw_2 | USER_DEFINED |
| 0x0080dc60 | Pitch | Global::Pitch_2 | USER_DEFINED |
| 0x0080dcf0 | pointto | Global::pointto | USER_DEFINED |
| 0x0080df50 | RotationArc | Global::RotationArc | USER_DEFINED |
| 0x0080e0c0 | polyhit | Global::polyhit | USER_DEFINED |
| 0x0080e4d0 | polyhit | Global::polyhit_2 | USER_DEFINED |
| 0x0080ec30 | BoxIntersect | Global::BoxIntersect | USER_DEFINED |
| 0x0080f240 | BoxIntersect | Global::BoxIntersect_2 | USER_DEFINED |
| 0x0080f6c0 | SphereIntersect | Global::SphereIntersect | USER_DEFINED |
| 0x0080f810 | equal | Global::equal | USER_DEFINED |
| 0x0080f870 | DistanceBetweenLines | Global::DistanceBetweenLines | USER_DEFINED |
| 0x0080fc90 | ClosestDistanceBetweenSegments | Global::ClosestDistanceBetweenSegments | USER_DEFINED |
| 0x00810220 | MakeMatrix4x4 | Global::MakeMatrix4x4 | USER_DEFINED |
| 0x008103a0 | Homogenize | Global::Homogenize | USER_DEFINED |
| 0x008103e0 | InterpolateBezier | Global::InterpolateBezier | USER_DEFINED |
| 0x00810660 | InterpolateBezier | Global::InterpolateBezier_2 | USER_DEFINED |
| 0x00810750 | IntersectLineSegments2d | Global::IntersectLineSegments2d | USER_DEFINED |
| 0x00811150 | IntersectCircle2d | Global::IntersectCircle2d | USER_DEFINED |
| 0x008115a0 | CSWNameGen::~CSWNameGen | CSWNameGen::Destructor_2 | USER_DEFINED |
| 0x00811fa0 | CSWNameGen::GetRandomName | CSWNameGen::GetRandomName_2 | USER_DEFINED |
| 0x00812ef0 | missing | Global::missing | USER_DEFINED |
| 0x00812ff0 | CResHelper<CResLTR,2036>::SetResRef | CResHelper_CResLTR_2036_::SetResRef | USER_DEFINED |
| 0x008131b0 | CResHelper<CResLTR,2036>::~CResHelper | CResHelper_CResLTR_2036_::Destructor | USER_DEFINED |
| 0x008132c0 | CResLTR::~CResLTR | CResLTR::Destructor_2 | USER_DEFINED |
| 0x00813fc0 | AurTextureSetDynamicMemoryRatio | Global::AurTextureSetDynamicMemoryRatio | USER_DEFINED |
| 0x00814000 | InsertionSortTexture | Global::InsertionSortTexture | USER_DEFINED |
| 0x008140a0 | AurGetImageWrapper | Global::AurGetImageWrapper | USER_DEFINED |
| 0x008142c0 | AurGetPLTImageWrapper | Global::AurGetPLTImageWrapper | USER_DEFINED |
| 0x00814310 | AurClearPalette | Global::AurClearPalette | USER_DEFINED |
| 0x00814340 | AurFreeImageWrapper | Global::AurFreeImageWrapper | USER_DEFINED |
| 0x008143c0 | AurTextureGetControllerFromReferencedBasic | Global::AurTextureGetControllerFromReferencedBasic | USER_DEFINED |
| 0x008143f0 | AurTextureBindTID | Global::AurTextureBindTID | USER_DEFINED |
| 0x008144e0 | AurTextureBindTargetIndex | Global::AurTextureBindTargetIndex | USER_DEFINED |
| 0x008145d0 | CAurTexture_scalar_deleting_destructor | Global::CAurTexture_scalar_deleting_destructor | USER_DEFINED |
| 0x00815ac0 | CAurTextureBasic_scalar_deleting_destructor | Global::CAurTextureBasic_scalar_deleting_destructor | USER_DEFINED |
| 0x00815e40 | AurTextureCalculateHashID | Global::AurTextureCalculateHashID | USER_DEFINED |
| 0x008170a0 | PerlinTextureController::~PerlinTextureController | PerlinTextureController::Destructor_2 | USER_DEFINED |
| 0x00817120 | RandomTextureController::~RandomTextureController | RandomTextureController::Destructor_2 | USER_DEFINED |
| 0x008188f0 | CAurTextureBasicEnsureMinimumImageShape | Global::CAurTextureBasicEnsureMinimumImageShape | USER_DEFINED |
| 0x008190f0 | AurTextureFind | Global::AurTextureFind | USER_DEFINED |
| 0x008192e0 | AurTextureGetTID | Global::AurTextureGetTID | USER_DEFINED |
| 0x00819330 | AurTextureMakeCurrent | Global::AurTextureMakeCurrent | USER_DEFINED |
| 0x00819360 | AurTextureSetLayered | Global::AurTextureSetLayered | USER_DEFINED |
| 0x00819400 | AurTextureReplaceSingleReference | Global::AurTextureReplaceSingleReference | USER_DEFINED |
| 0x00819620 | AurTextureReplaceTileReferences | Global::AurTextureReplaceTileReferences | USER_DEFINED |
| 0x00819980 | AurTextureSetCubeMapParameter | Global::AurTextureSetCubeMapParameter | USER_DEFINED |
| 0x00819a30 | AurTextureGetNormalizationCubeMap | Global::AurTextureGetNormalizationCubeMap | USER_DEFINED |
| 0x00819b10 | AurTextureGetNULL | Global::AurTextureGetNULL | USER_DEFINED |
| 0x00819be0 | AurTextureGetReference | Global::AurTextureGetReference | USER_DEFINED |
| 0x00819c20 | AurTextureGet | Global::AurTextureGet | USER_DEFINED |
| 0x00819da0 | AurLiteReleaseFunctionHolder | Global::AurLiteReleaseFunctionHolder | USER_DEFINED |
| 0x00819e40 | AurTextureSetAttributes | Global::AurTextureSetAttributes | USER_DEFINED |
| 0x00819ea0 | AurTextureMessage | Global::AurTextureMessage | USER_DEFINED |
| 0x00819f00 | AurTextureBuildAndStoreAll | Global::AurTextureBuildAndStoreAll | USER_DEFINED |
| 0x0081a060 | AurTextureGetOrdering | Global::AurTextureGetOrdering | USER_DEFINED |
| 0x0081a150 | AurTextureGetMaxTexID | Global::AurTextureGetMaxTexID | USER_DEFINED |
| 0x0081a160 | AurTextureCalculateNewTexMem | Global::AurTextureCalculateNewTexMem | USER_DEFINED |
| 0x0081a250 | AurInternalResourceBarrier | Global::AurInternalResourceBarrier | USER_DEFINED |
| 0x0081a260 | AurTextureMemCheck | Global::AurTextureMemCheck | USER_DEFINED |
| 0x0081a490 | AurTextureResetAll | Global::AurTextureResetAll | USER_DEFINED |
| 0x0081a4e0 | AurTextureSetMem | Global::AurTextureSetMem | USER_DEFINED |
| 0x0081a630 | AurTextureMakeCurrent | Global::AurTextureMakeCurrent_2 | USER_DEFINED |
| 0x0081a740 | AurTextureForceControllerUpdate | Global::AurTextureForceControllerUpdate | USER_DEFINED |
| 0x0081b650 | AurTextureSetModeStatic | Global::AurTextureSetModeStatic | USER_DEFINED |
| 0x0081b660 | AurTextureSetModeDynamic | Global::AurTextureSetModeDynamic | USER_DEFINED |
| 0x0081b670 | AurTextureEnvMapDisabled | Global::AurTextureEnvMapDisabled | USER_DEFINED |
| 0x0081b680 | SafePointerList<CAurTexture>::~SafePointerList | SafePointerList_CAurTexture_::Destructor | USER_DEFINED |
| 0x0081b740 | List<CAurTextureBasic*>::Insert | List_CAurTextureBasic__::Insert | USER_DEFINED |
| 0x0081b7a0 | Parse | Global::Parse | USER_DEFINED |
| 0x0081b990 | List<CAurTextureBasic*>::AddUnique | List_CAurTextureBasic__::AddUnique | USER_DEFINED |
| 0x0081ba40 | List<T*>::allocate | List_T__::allocate | USER_DEFINED |
| 0x0081bb20 | InitializeSystemPoolManager | Global::InitializeSystemPoolManager | USER_DEFINED |
| 0x0081bbb0 | InitializeVertexBufferObjectPoolManager | Global::InitializeVertexBufferObjectPoolManager | USER_DEFINED |
| 0x00825ec0 | AurBuild2DMipMaps | Global::AurBuild2DMipMaps | USER_DEFINED |
| 0x00828ca0 | GLRender::AddSquareToRender | GLRender::AddSquareToRender_2 | USER_DEFINED |
| 0x00828cc0 | GLRender::RemoveSquareToRender | GLRender::RemoveSquareToRender_2 | USER_DEFINED |
| 0x0083b980 | UpdateAnimFootprint | Global::UpdateAnimFootprint | USER_DEFINED |
| 0x0083d740 | SynchronizeNodes | Global::SynchronizeNodes | USER_DEFINED |
| 0x0083d8a0 | GetMaxID | Global::GetMaxID | USER_DEFINED |
| 0x00843950 | List<T>::~List | List_T_::Destructor | USER_DEFINED |
| 0x00843e00 | List<PartEmitter*>::allocate | List_PartEmitter__::allocate | USER_DEFINED |
| 0x00844500 | MaxTreeDestructor | Global::MaxTreeDestructor | USER_DEFINED |
| 0x00844510 | MaxTreeParseField | Global::MaxTreeParseField | USER_DEFINED |
| 0x00844e00 | MaxTree::FreeTheFreeList | MaxTree::FreeTheFreeList_2 | USER_DEFINED |
| 0x00844ea0 | ModelDestructor | Global::ModelDestructor | USER_DEFINED |
| 0x00844eb0 | ModelParseField | Global::ModelParseField | USER_DEFINED |
| 0x00846720 | UpdateAnimFootprint | Global::UpdateAnimFootprint_2 | USER_DEFINED |
| 0x008471e0 | MdlNode::AsMdlNodeCamera | MdlNode::AsMdlNodeCamera_2 | USER_DEFINED |
| 0x00847270 | MdlNode::AsMdlNodeReference | MdlNode::AsMdlNodeReference_2 | USER_DEFINED |
| 0x00847370 | MdlNode::AsMdlNodeLightsaber | MdlNode::AsMdlNodeLightsaber_2 | USER_DEFINED |
| 0x008477c0 | MdlNode::DestructorDispatch | MdlNode::DestructorDispatch_2 | USER_DEFINED |
| 0x00847850 | Model::InternalDestructor | Model::InternalDestructor_2 | USER_DEFINED |
| 0x00847bb0 | MdlNode::MdlNode | MdlNode::Constructor_2 | USER_DEFINED |
| 0x00847cb0 | MdlNode::AddToFreeList | MdlNode::AddToFreeList_2 | USER_DEFINED |
| 0x00847ee0 | PartLightsaber::~PartLightsaber | PartLightsaber::Destructor_2 | USER_DEFINED |
| 0x00847f20 | MdlNodeDanglyMeshRemoveTemporaryArrays | Global::MdlNodeDanglyMeshRemoveTemporaryArrays | USER_DEFINED |
| 0x00847f30 | MdlNodeDanglyMeshGenVertices | Global::MdlNodeDanglyMeshGenVertices | USER_DEFINED |
| 0x00848260 | PartDanglyMesh::~PartDanglyMesh | PartDanglyMesh::Destructor_2 | USER_DEFINED |
| 0x00848320 | PartSkin::~PartSkin | PartSkin::Destructor_2 | USER_DEFINED |
| 0x00848d80 | PartReference::~PartReference | PartReference::Destructor_2 | USER_DEFINED |
| 0x00849110 | FindPart | Global::FindPart | USER_DEFINED |
| 0x008491f0 | CreateInstanceTreeR | Global::CreateInstanceTreeR | USER_DEFINED |
| 0x008492e0 | CreateInstanceTree | Global::CreateInstanceTree | USER_DEFINED |
| 0x00849480 | Part::~Part | Part::Destructor_2 | USER_DEFINED |
| 0x00849870 | PartTriMesh::~PartTriMesh | PartTriMesh::Destructor_2 | USER_DEFINED |
| 0x0084a470 | ModelParseField | Global::ModelParseField_2 | USER_DEFINED |
| 0x0084af50 | PartLight::~PartLight | PartLight::Destructor_2 | USER_DEFINED |
| 0x0084b050 | FindGob | Global::FindGob | USER_DEFINED |
| 0x0084bb00 | CAurPathInfo::~CAurPathInfo | CAurPathInfo::Destructor_2 | USER_DEFINED |
| 0x0084bb80 | return_void_0x4 | Global::return_void_0x4 | USER_DEFINED |
| 0x0084c4e0 | PartSetFog | Global::PartSetFog | USER_DEFINED |
| 0x0084c770 | Gob::~Gob | Gob::Destructor_2 | USER_DEFINED |
| 0x0084e510 | PartHasMaterial | Global::PartHasMaterial | USER_DEFINED |
| 0x0084f670 | AnimMeshHitCheck | Global::AnimMeshHitCheck | USER_DEFINED |
| 0x00850530 | HitCheckAABBnode | Global::HitCheckAABBnode | USER_DEFINED |
| 0x00852370 | PartInitMaterial | Global::PartInitMaterial | USER_DEFINED |
| 0x008525d0 | Gob::GetMinimumBoundingBox | Gob::GetMinimumBoundingBox_2 | USER_DEFINED |
| 0x008531b0 | Gob::ReplaceTexture | Gob::ReplaceTexture_2 | USER_DEFINED |
| 0x00853600 | PartSetCanDownSample | Global::PartSetCanDownSample | USER_DEFINED |
| 0x008536b0 | SetCanDownSample | Global::SetCanDownSample | USER_DEFINED |
| 0x00853700 | GetAllNamedTextures | Global::GetAllNamedTextures | USER_DEFINED |
| 0x00854030 | Gob::SetScene | Gob::SetScene_2 | USER_DEFINED |
| 0x00854100 | TreeHasSingleTexture | Global::TreeHasSingleTexture | USER_DEFINED |
| 0x008541f0 | TreeHasTransparency | Global::TreeHasTransparency | USER_DEFINED |
| 0x00854280 | TreeHasGeometry | Global::TreeHasGeometry | USER_DEFINED |
| 0x00854300 | TreeSetGeometryFlags | Global::TreeSetGeometryFlags | USER_DEFINED |
| 0x00854480 | CreateReferenceObjects | Global::CreateReferenceObjects | USER_DEFINED |
| 0x008548b0 | NewCAurObject | Global::NewCAurObject | USER_DEFINED |
| 0x008556b0 | Gob::CollectLights | Gob::CollectLights_2 | USER_DEFINED |
| 0x00855c90 | Gob::EnumParts | Gob::EnumParts_2 | USER_DEFINED |
| 0x00855df0 | AurControlPart | Global::AurControlPart | USER_DEFINED |
| 0x00856330 | CExoArrayList::CExoArrayList | CExoArrayList::Constructor_2 | USER_DEFINED |
| 0x00856fd0 | CAurScene_scalar_deleting_destructor | Global::CAurScene_scalar_deleting_destructor | USER_DEFINED |
| 0x00857120 | Scene_scalar_deleting_destructor | Global::Scene_scalar_deleting_destructor | USER_DEFINED |
| 0x00857610 | FindScene | Global::FindScene | USER_DEFINED |
| 0x00857ae0 | CHitInfo::Reset | CHitInfo::Reset_2 | USER_DEFINED |
| 0x00857df0 | Scene::ListSpecialPartsR | Scene::ListSpecialPartsR_2 | USER_DEFINED |
| 0x00858070 | ProjectDynamicShadow | Global::ProjectDynamicShadow | USER_DEFINED |
| 0x00859490 | GobOrderCmp | Global::GobOrderCmp | USER_DEFINED |
| 0x00859710 | EmitterOrderCmp | Global::EmitterOrderCmp | USER_DEFINED |
| 0x00859b30 | AurEnableInterleavedAnimations | Global::AurEnableInterleavedAnimations | USER_DEFINED |
| 0x00859b40 | AurDisableInterleavedAnimations | Global::AurDisableInterleavedAnimations | USER_DEFINED |
| 0x00859b50 | Gob::VisibilityCheck | Gob::VisibilityCheck_2 | USER_DEFINED |
| 0x0085aa60 | Scene::DoBlurBuckets | Scene::DoBlurBuckets_2 | USER_DEFINED |
| 0x0085be40 | SetShadowStencilBuffer | Global::SetShadowStencilBuffer | USER_DEFINED |
| 0x0085c850 | AurDrawVectorList | Global::AurDrawVectorList | USER_DEFINED |
| 0x0085e0e0 | RenderPersonalSpace | Global::RenderPersonalSpace | USER_DEFINED |
| 0x0085e270 | RenderPaths | Global::RenderPaths | USER_DEFINED |
| 0x0085f600 | NewCAurScene | Global::NewCAurScene | USER_DEFINED |
| 0x0085fd80 | ComputePartBBox | Global::ComputePartBBox | USER_DEFINED |
| 0x00860bf0 | AurTextureSetModeStatic | Global::AurTextureSetModeStatic_2 | USER_DEFINED |
| 0x00860c70 | CollapsePartTree | Global::CollapsePartTree | USER_DEFINED |
| 0x00861850 | ExpandReferenceParts | Global::ExpandReferenceParts | USER_DEFINED |
| 0x00861920 | SetPartTextureRotate | Global::SetPartTextureRotate | USER_DEFINED |
| 0x008619b0 | ProcessTilePart | Global::ProcessTilePart | USER_DEFINED |
| 0x00861ce0 | CAurRoom*Scene::FindRoom | CAurRoom_Scene::FindRoom | USER_DEFINED |
| 0x008621f0 | Scene::SetCurrentRoom | Scene::SetCurrentRoom_2 | USER_DEFINED |
| 0x00862f00 | SpawnPartsForTile | Global::SpawnPartsForTile | USER_DEFINED |
| 0x008630e0 | ProcessStaticParts | Global::ProcessStaticParts | USER_DEFINED |
| 0x008646a0 | Scene::RemoveFromSceneManager | Scene::RemoveFromSceneManager_2 | USER_DEFINED |
| 0x008646f0 | AurSetGlobalWind | Global::AurSetGlobalWind | USER_DEFINED |
| 0x00864770 | AurSetGlobalWindVariation | Global::AurSetGlobalWindVariation | USER_DEFINED |
| 0x00865e90 | List<T*>::Add | List_T__::Add | USER_DEFINED |
| 0x00865f80 | List<T>::SetSize | List_T_::SetSize | USER_DEFINED |
| 0x008660c0 | SafePointer<T>::operator= | SafePointer_T_::operator= | USER_DEFINED |
| 0x008663a0 | AurInternalRenderStateShutdown | Global::AurInternalRenderStateShutdown | USER_DEFINED |
| 0x008668b0 | AurInternalRenderStringShutdown | Global::AurInternalRenderStringShutdown | USER_DEFINED |
| 0x00866940 | PostTraceStrings | Global::PostTraceStrings | USER_DEFINED |
| 0x00866a10 | untrace | Global::untrace | USER_DEFINED |
| 0x00866af0 | AurRenderStrings | Global::AurRenderStrings | USER_DEFINED |
| 0x00866b40 | AurPostString | Other::AurPostString, Global::AurPostString | USER_DEFINED |
| 0x00867040 | CommandCompletion | Global::CommandCompletion | USER_DEFINED |
| 0x00867210 | FuncInterp | Global::FuncInterp | USER_DEFINED |
| 0x00867660 | FuncPreInterp | Global::FuncPreInterp | USER_DEFINED |
| 0x008677e0 | ConsoleFunc::ConsoleFunc | ConsoleFunc::StringConstructor, ConsoleFunc::Constructor_2 | USER_DEFINED |
| 0x00867880 | ConsoleFunc::ConsoleFunc | ConsoleFunc::IntConstructor, ConsoleFunc::Constructor_3 | USER_DEFINED |
| 0x00867a20 | AurResGet | Global::AurResGet | USER_DEFINED |
| 0x00867cc0 | AurResGet | Global::AurResGet_2 | USER_DEFINED |
| 0x00867dc0 | GetResource | Global::GetResource | USER_DEFINED |
| 0x00867e10 | AurResFree | Global::AurResFree | USER_DEFINED |
| 0x00867ee0 | AurResGetNextLine | Global::AurResGetNextLine | USER_DEFINED |
| 0x00867fa0 | AurResFree | Global::AurResFree_2 | USER_DEFINED |
| 0x00868080 | AurResGetNextLine | Global::AurResGetNextLine_2 | USER_DEFINED |
| 0x00868120 | AurResGetData | Global::AurResGetData | USER_DEFINED |
| 0x00868260 | AurResGetDataBytes | Global::AurResGetDataBytes | USER_DEFINED |
| 0x008682e0 | AurResFreeDataBytes | Global::AurResFreeDataBytes | USER_DEFINED |
| 0x00868320 | dofile | Global::dofile | USER_DEFINED |
| 0x00868590 | AurEnableHeadLight | Global::AurEnableHeadLight | USER_DEFINED |
| 0x008685c0 | AurDisableHeadLight | Global::AurDisableHeadLight | USER_DEFINED |
| 0x0086a8c0 | NewCAurora | Global::NewCAurora | USER_DEFINED |
| 0x0086a8d0 | DestroyCAurora | Global::DestroyCAurora | USER_DEFINED |
| 0x0086a900 | AurMakeCurrent | Global::AurMakeCurrent | USER_DEFINED |
| 0x0086a970 | AurAddDrawableSquare | Global::AurAddDrawableSquare | USER_DEFINED |
| 0x0086ab20 | AurRemoveDrawableSquare | Global::AurRemoveDrawableSquare | USER_DEFINED |
| 0x0086ab70 | AurInitExtensions | Global::AurInitExtensions | USER_DEFINED |
| 0x0086ab80 | AurEnableShadows | Global::AurEnableShadows | USER_DEFINED |
| 0x0086ab90 | AurDisableShadows | Global::AurDisableShadows | USER_DEFINED |
| 0x0086abb0 | AurEnableGrass | Global::AurEnableGrass | USER_DEFINED |
| 0x0086abc0 | AurDisableGrass | Global::AurDisableGrass | USER_DEFINED |
| 0x0086abd0 | AurEnableSoftShadows | Global::AurEnableSoftShadows | USER_DEFINED |
| 0x0086abf0 | AurDisableSoftShadows | Global::AurDisableSoftShadows | USER_DEFINED |
| 0x0086ac00 | AurEnableFrameBufferEffects | Global::AurEnableFrameBufferEffects | USER_DEFINED |
| 0x0086ac10 | AurDisableFrameBufferEffects | Global::AurDisableFrameBufferEffects | USER_DEFINED |
| 0x0086ac20 | AurQueryMaximumAnisotropy | Global::AurQueryMaximumAnisotropy | USER_DEFINED |
| 0x0086ac30 | AurSetAnisotropy | Global::AurSetAnisotropy | USER_DEFINED |
| 0x0086ac50 | AurSetVSyncState | Global::AurSetVSyncState | USER_DEFINED |
| 0x0086ac80 | AurSetPauseOverlayColor | Global::AurSetPauseOverlayColor | USER_DEFINED |
| 0x0086ad30 | AurIsSpeedBlurEnabled | Global::AurIsSpeedBlurEnabled | USER_DEFINED |
| 0x0086ad40 | AurEnableSpeedBlur | Global::AurEnableSpeedBlur | USER_DEFINED |
| 0x0086ad60 | AurDisableSpeedBlur | Global::AurDisableSpeedBlur | USER_DEFINED |
| 0x0086ad80 | AurEnableSaturation | Global::AurEnableSaturation | USER_DEFINED |
| 0x0086ad90 | AurDisableSaturation | Global::AurDisableSaturation | USER_DEFINED |
| 0x0086ada0 | AurEnableScanNoise | Global::AurEnableScanNoise | USER_DEFINED |
| 0x0086adb0 | AurDisableScanNoise | Global::AurDisableScanNoise | USER_DEFINED |
| 0x0086adc0 | AurEnableClairvoyance | Global::AurEnableClairvoyance | USER_DEFINED |
| 0x0086ade0 | AurDisableClairvoyance | Global::AurDisableClairvoyance | USER_DEFINED |
| 0x0086adf0 | AurEnableForceSight | Global::AurEnableForceSight | USER_DEFINED |
| 0x0086ae20 | AurDisableForceSight | Global::AurDisableForceSight | USER_DEFINED |
| 0x0086ae30 | AurEnableForceFury | Global::AurEnableForceFury | USER_DEFINED |
| 0x0086ae50 | AurSetSaturationParams | Global::AurSetSaturationParams | USER_DEFINED |
| 0x0086ae80 | AurSetSpeedBlurRatio | Global::AurSetSpeedBlurRatio | USER_DEFINED |
| 0x0086aee0 | AurSetAntiAliasingMode | Global::AurSetAntiAliasingMode | USER_DEFINED |
| 0x0086af00 | AurQueryAntiAliasingMode | Global::AurQueryAntiAliasingMode | USER_DEFINED |
| 0x0086b0b0 | AurDisableVertexBufferObjects | Global::AurDisableVertexBufferObjects | USER_DEFINED |
| 0x0086b0c0 | AurDisableVertexBufferObjectsWriteOnly | Global::AurDisableVertexBufferObjectsWriteOnly | USER_DEFINED |
| 0x0086b0d0 | AurEnableVertexBufferObjectsWriteOnly | Global::AurEnableVertexBufferObjectsWriteOnly | USER_DEFINED |
| 0x0086b0e0 | AurATIUnlockSoftShadowsForHotFix | Global::AurATIUnlockSoftShadowsForHotFix | USER_DEFINED |
| 0x0086b100 | AurSetClearColor | Global::AurSetClearColor | USER_DEFINED |
| 0x0086b130 | AurGetClearColor | Global::AurGetClearColor | USER_DEFINED |
| 0x0086b150 | AurATIFragmentShaderAvailable | Global::AurATIFragmentShaderAvailable | USER_DEFINED |
| 0x0086b190 | AurBumpMapAvailable | Global::AurBumpMapAvailable | USER_DEFINED |
| 0x0086b210 | AurNV1xLevelShadersAvailable | Global::AurNV1xLevelShadersAvailable | USER_DEFINED |
| 0x0086b250 | AurNV2xLevelShadersAvailable | Global::AurNV2xLevelShadersAvailable | USER_DEFINED |
| 0x0086b270 | AurBumpyShinyAvailable | Global::AurBumpyShinyAvailable | USER_DEFINED |
| 0x0086b2a0 | AurSwapControlAvailable | Global::AurSwapControlAvailable | USER_DEFINED |
| 0x0086b2e0 | AurBumpMapCardValidate | Global::AurBumpMapCardValidate | USER_DEFINED |
| 0x0086b330 | AurATIValidate | Global::AurATIValidate | USER_DEFINED |
| 0x0086b420 | AurOnePassEnvMapAvailable | Global::AurOnePassEnvMapAvailable | USER_DEFINED |
| 0x0086b4a0 | AurMultiTextureAvailable | Global::AurMultiTextureAvailable | USER_DEFINED |
| 0x0086b4e0 | AurVertexProgramsNVAvailable | Global::AurVertexProgramsNVAvailable | USER_DEFINED |
| 0x0086b520 | AurVertexBufferObjectARBAvailable | Global::AurVertexBufferObjectARBAvailable | USER_DEFINED |
| 0x0086b570 | AurVertexProgramsAvailable | Global::AurVertexProgramsAvailable | USER_DEFINED |
| 0x0086b5b0 | AurSoftShadowsAvailable | Global::AurSoftShadowsAvailable | USER_DEFINED |
| 0x0086b5e0 | AurATISoftShadowsAvailable | Global::AurATISoftShadowsAvailable | USER_DEFINED |
| 0x0086b640 | AurFrameBufferModificationsAvailable | Global::AurFrameBufferModificationsAvailable | USER_DEFINED |
| 0x0086b6a0 | AurATIFrameBufferModificationsAvailable | Global::AurATIFrameBufferModificationsAvailable | USER_DEFINED |
| 0x0086b700 | AurCubeMapAvailable | Global::AurCubeMapAvailable | USER_DEFINED |
| 0x0086b770 | AurAnisotropicFilterAvailable | Global::AurAnisotropicFilterAvailable | USER_DEFINED |
| 0x0086b7b0 | AurTextureCompressionAvailable | Global::AurTextureCompressionAvailable | USER_DEFINED |
| 0x0086b870 | AurTextureS3TCAvailable | Global::AurTextureS3TCAvailable | USER_DEFINED |
| 0x0086b8b0 | CheckExtension | Global::CheckExtension | USER_DEFINED |
| 0x0086b940 | AurGetStencilClearValue | Global::AurGetStencilClearValue | USER_DEFINED |
| 0x0086b970 | AurTwoSidedStencilAvailable | Global::AurTwoSidedStencilAvailable | USER_DEFINED |
| 0x0086b9f0 | AurSGIMipmapAvailable | Global::AurSGIMipmapAvailable | USER_DEFINED |
| 0x0086ba10 | AurCheckFrameBufferModificationsAvailable | Global::AurCheckFrameBufferModificationsAvailable | USER_DEFINED |
| 0x0086ba30 | AuroraUseARBShaders | Global::AuroraUseARBShaders | USER_DEFINED |
| 0x0086ba60 | AurClearActiveBins | Global::AurClearActiveBins | USER_DEFINED |
| 0x0086ba80 | PartOutside | Global::PartOutside | USER_DEFINED |
| 0x0086bb10 | PartOutside | Global::PartOutside_2 | USER_DEFINED |
| 0x0086bbc0 | PartOutside | Global::PartOutside_3 | USER_DEFINED |
| 0x0086bc40 | PartOutside | Global::PartOutside_4 | USER_DEFINED |
| 0x0086bcc0 | GobOutside | Global::GobOutside | USER_DEFINED |
| 0x0086bf50 | BinOutside | Global::BinOutside | USER_DEFINED |
| 0x0086bfc0 | AddPartToMeshBuckets | Global::AddPartToMeshBuckets | USER_DEFINED |
| 0x0086c060 | LightPartTriMesh | Global::LightPartTriMesh | USER_DEFINED |
| 0x0086d070 | ClearBuckets | Global::ClearBuckets | USER_DEFINED |
| 0x0086d3e0 | BSPNodeEnsureNodeData | Global::BSPNodeEnsureNodeData | USER_DEFINED |
| 0x0086d460 | SceneBSPGetTileData | Global::SceneBSPGetTileData | USER_DEFINED |
| 0x0086d4b0 | SceneBSPGetObjectTextures | Global::SceneBSPGetObjectTextures | USER_DEFINED |
| 0x0086d530 | SceneBSPGetTileTextures | Global::SceneBSPGetTileTextures | USER_DEFINED |
| 0x0086d5f0 | SceneBSPAddTileData | Global::SceneBSPAddTileData | USER_DEFINED |
| 0x0086d690 | SceneBSPRemoveTiledata | Global::SceneBSPRemoveTiledata | USER_DEFINED |
| 0x0086d830 | SceneBSPAddGob | Global::SceneBSPAddGob | USER_DEFINED |
| 0x0086d8c0 | SceneBSPRemoveGob | Global::SceneBSPRemoveGob | USER_DEFINED |
| 0x0086d950 | NodeHasGob | Global::NodeHasGob | USER_DEFINED |
| 0x0086d990 | SceneBSPAddPart | Global::SceneBSPAddPart | USER_DEFINED |
| 0x0086de00 | SceneBSPRemovePart | Global::SceneBSPRemovePart | USER_DEFINED |
| 0x0086e1f0 | NodeHasLight | Global::NodeHasLight | USER_DEFINED |
| 0x0086e220 | NodeHasEmitter | Global::NodeHasEmitter | USER_DEFINED |
| 0x0086e260 | NodeHasProjection | Global::NodeHasProjection | USER_DEFINED |
| 0x0086e2c0 | NodeHasTriMesh | Global::NodeHasTriMesh | USER_DEFINED |
| 0x0086e300 | NodeHasTrigger | Global::NodeHasTrigger | USER_DEFINED |
| 0x0086e340 | NodeHasAABB | Global::NodeHasAABB | USER_DEFINED |
| 0x0086e380 | NodeHasOther | Global::NodeHasOther | USER_DEFINED |
| 0x0086e3c0 | DeleteNonGobParts | Global::DeleteNonGobParts | USER_DEFINED |
| 0x0086e830 | HitPartOnBSPNode | Global::HitPartOnBSPNode | USER_DEFINED |
| 0x0086eba0 | LightBins | Global::LightBins | USER_DEFINED |
| 0x0086f110 | LightNodeTile | Global::LightNodeTile | USER_DEFINED |
| 0x0086f770 | DoStaticLightBuckets | Global::DoStaticLightBuckets | USER_DEFINED |
| 0x0086feb0 | AnimateStaticLight | Global::AnimateStaticLight | USER_DEFINED |
| 0x0086fef0 | UpdateStaticLighting | Global::UpdateStaticLighting | USER_DEFINED |
| 0x0086ffc0 | CollectBins | Global::CollectBins | USER_DEFINED |
| 0x00870230 | AurProcessActiveBins | Global::AurProcessActiveBins | USER_DEFINED |
| 0x00870360 | AddLightToRooms | Global::AddLightToRooms | USER_DEFINED |
| 0x00870d60 | SceneAddPartToRooms | Global::SceneAddPartToRooms | USER_DEFINED |
| 0x00870ea0 | SceneRemovePartFromRooms | Global::SceneRemovePartFromRooms | USER_DEFINED |
| 0x00871000 | ManageSceneBSP | Global::ManageSceneBSP | USER_DEFINED |
| 0x008717c0 | AddObjectToRooms | Global::AddObjectToRooms | USER_DEFINED |
| 0x00872fd0 | List<PartLight*>::AddUnique | List_PartLight__::AddUnique | USER_DEFINED |
| 0x00873000 | List<T*>::operator= | List_T__::operator= | USER_DEFINED |
| 0x00873050 | List<T*>::List | List_T__::List | USER_DEFINED |
| 0x008730c0 | List<T*>::Remove | List_T__::Remove | USER_DEFINED |
| 0x00873110 | List<T*>::Count | List_T__::Count | USER_DEFINED |
| 0x008731c0 | SafePointerList<T>::AddPointer | SafePointerList_T_::AddPointer | USER_DEFINED |
| 0x00873f10 | GatherBones | Global::GatherBones | USER_DEFINED |
| 0x008742a0 | MakeInlinePartList | Global::MakeInlinePartList | USER_DEFINED |
| 0x008742f0 | FindPartIndexDFS | Global::FindPartIndexDFS | USER_DEFINED |
| 0x008767f0 | ExecuteSkinTransforms | Global::ExecuteSkinTransforms | USER_DEFINED |
| 0x00877a20 | RenderSkinned | Global::RenderSkinned | USER_DEFINED |
| 0x00878d00 | GetSkinVP | Global::GetSkinVP | USER_DEFINED |
| 0x00878da0 | GetSkinCubeMapVP | Global::GetSkinCubeMapVP | USER_DEFINED |
| 0x00878e50 | RenderSkinnedVP | Global::RenderSkinnedVP | USER_DEFINED |
| 0x0087f060 | RenderSkinnedVPBlur | Global::RenderSkinnedVPBlur | USER_DEFINED |
| 0x00882ef0 | List<SkinVertexWeight>::Add | List_SkinVertexWeight_::Add | USER_DEFINED |
| 0x00882f70 | List<SkinVertexWeight>::operator= | List_SkinVertexWeight_::operator= | USER_DEFINED |
| 0x00883010 | List<Matrix>::Add | List_Matrix_::Add | USER_DEFINED |
| 0x00883090 | List<Matrix>::allocate | List_Matrix_::allocate | USER_DEFINED |
| 0x008836b0 | PartTriMesh::GetTextureAngle | PartTriMesh::GetTextureAngle_2 | USER_DEFINED |
| 0x008837f0 | PartTriMesh::GetMinimumSphere | PartTriMesh::GetMinimumSphere_2 | USER_DEFINED |
| 0x00883b00 | RenderFlat | Global::RenderFlat | USER_DEFINED |
| 0x00883f00 | RenderLightMappedNoShaders | Global::RenderLightMappedNoShaders | USER_DEFINED |
| 0x00887c30 | RenderEnvironmentMapped | Global::RenderEnvironmentMapped | USER_DEFINED |
| 0x00888430 | RenderEMLM | Global::RenderEMLM | USER_DEFINED |
| 0x008887c0 | RenderNV1xEMLM | Global::RenderNV1xEMLM | USER_DEFINED |
| 0x0088c580 | RenderDiffuseBumpMappedPass | Global::RenderDiffuseBumpMappedPass | USER_DEFINED |
| 0x0088dde0 | RenderFlatVP | Global::RenderFlatVP | USER_DEFINED |
| 0x0088e030 | RenderLightmappedVP | Global::RenderLightmappedVP | USER_DEFINED |
| 0x0088e150 | RenderEnvironmentMappedVP | Global::RenderEnvironmentMappedVP | USER_DEFINED |
| 0x0088e2d0 | RenderEMLMVP | Global::RenderEMLMVP | USER_DEFINED |
| 0x0088e450 | RenderBumpmapFallback | Global::RenderBumpmapFallback | USER_DEFINED |
| 0x008902e0 | SetBeaming | Global::SetBeaming | USER_DEFINED |
| 0x00897390 | PartTriMesh::Mat | PartTriMesh::Mat_2 | USER_DEFINED |
| 0x00897eb0 | ClipProjection | Global::ClipProjection | USER_DEFINED |
| 0x0089b230 | AurGrassMaterialInUse | Global::AurGrassMaterialInUse | USER_DEFINED |
| 0x0089b270 | AurGrassClearInfo | Global::AurGrassClearInfo | USER_DEFINED |
| 0x0089b280 | AurGrassAddInfo | Global::AurGrassAddInfo | USER_DEFINED |
| 0x0089b8e8 | PartTriMesh_Part_vtable_0x0C_gt_GetWorld_Vector_ref_Quaternion_ref | Global::PartTriMesh_Part_vtable_0x0C_gt_GetWorld_Vector_ref_Quaternion_ref | USER_DEFINED |
| 0x0089c9c0 | CAurTriangleBin::RenderGrassPolys | CAurTriangleBin::RenderGrassPolys_2 | USER_DEFINED |
| 0x0089d620 | GrassSortTris | Global::GrassSortTris | USER_DEFINED |
| 0x008a0d90 | SetEmitterTarget | Global::SetEmitterTarget | USER_DEFINED |
| 0x008a0e30 | Detonate | Global::Detonate | USER_DEFINED |
| 0x008a13b0 | Particle::~Particle | Particle::Destructor_2 | USER_DEFINED |
| 0x008a44a0 | ChunkyParticle::~ChunkyParticle | ChunkyParticle::Destructor_2 | USER_DEFINED |
| 0x008b6830 | PartEmitter::~PartEmitter | PartEmitter::Destructor_2 | USER_DEFINED |
| 0x008b7610 | LightningEmitter::~LightningEmitter | LightningEmitter::Destructor_2 | USER_DEFINED |
| 0x008bfdd0 | List<T>::Add | List_T_::Add | USER_DEFINED |
| 0x008c0210 | Camera::~Camera | Camera::Destructor_2 | USER_DEFINED |
| 0x008c1f60 | Camera::HitCheck | Camera::HitCheck_2 | USER_DEFINED |
| 0x008c2290 | return_zero_0x10 | Global::return_zero_0x10 | USER_DEFINED |
| 0x008c2c40 | NewCAurCamera | Global::NewCAurCamera | USER_DEFINED |
| 0x008c2cc0 | List<Plane>::List | List_Plane_::List | USER_DEFINED |
| 0x008c2d00 | List<Plane>::allocate | List_Plane_::allocate | USER_DEFINED |
| 0x008c2d60 | List<Plane>::do_allocate | List_Plane_::do_allocate | USER_DEFINED |
| 0x008c3320 | UpdateScenesAttach | Global::UpdateScenesAttach | USER_DEFINED |
| 0x008c33b0 | UpdateDependentPosition | Global::UpdateDependentPosition | USER_DEFINED |
| 0x008c33e0 | PropagateColorShifting | Global::PropagateColorShifting | USER_DEFINED |
| 0x008c3450 | PropagateIllumination | Global::PropagateIllumination | USER_DEFINED |
| 0x008c34b0 | PropagateRender | Global::PropagateRender | USER_DEFINED |
| 0x008c3520 | PropagateViewVolume | Global::PropagateViewVolume | USER_DEFINED |
| 0x008c46a0 | CAurBehaviorCameraFollow::~CAurBehaviorCameraFollow | CAurBehaviorCameraFollow::Destructor_2 | USER_DEFINED |
| 0x008c5220 | AurMessageSkipWhitespace | Global::AurMessageSkipWhitespace | USER_DEFINED |
| 0x008c5260 | AurMessageGetToken | Global::AurMessageGetToken | USER_DEFINED |
| 0x008c52e0 | AurMessageParseVector | Global::AurMessageParseVector | USER_DEFINED |
| 0x008c5310 | AurMessageParseString | Global::AurMessageParseString | USER_DEFINED |
| 0x008c5350 | AurMessageParseFloat | Global::AurMessageParseFloat | USER_DEFINED |
| 0x008c5370 | AurMessageParseIntOrBool | Global::AurMessageParseIntOrBool | USER_DEFINED |
| 0x008c5410 | Parse | Global::Parse_2 | USER_DEFINED |
| 0x008c5430 | Parse | Global::Parse_3 | USER_DEFINED |
| 0x008c5460 | Parse | Global::Parse_4 | USER_DEFINED |
| 0x008c5520 | Parse | Global::Parse_5 | USER_DEFINED |
| 0x008c5600 | Parse | Global::Parse_6 | USER_DEFINED |
| 0x008c5720 | Parse | Global::Parse_7 | USER_DEFINED |
| 0x008c5820 | Parse | Global::Parse_8 | USER_DEFINED |
| 0x008c5950 | Parse | Global::Parse_9 | USER_DEFINED |
| 0x008c59c0 | Parse | Global::Parse_10 | USER_DEFINED |
| 0x008c5f90 | Parse | Global::Parse_11 | USER_DEFINED |
| 0x008c6120 | Parse | Global::Parse_12 | USER_DEFINED |
| 0x008c61e0 | FindMdlNode | Global::FindMdlNode | USER_DEFINED |
| 0x008c6270 | FindMdlNode | Global::FindMdlNode_2 | USER_DEFINED |
| 0x008c62a0 | CompressQuaternionKey | Global::CompressQuaternionKey | USER_DEFINED |
| 0x008c6720 | AddGenericControllerSingle | Global::AddGenericControllerSingle | USER_DEFINED |
| 0x008c68e0 | CullUnnecessaryKeys | Global::CullUnnecessaryKeys | USER_DEFINED |
| 0x008c6c50 | AddGenericController | Global::AddGenericController | USER_DEFINED |
| 0x008c6e50 | AddBezierController | Global::AddBezierController | USER_DEFINED |
| 0x008cd280 | ParseNode | Global::ParseNode | USER_DEFINED |
| 0x008ce510 | BinarySearchModel | Global::BinarySearchModel | USER_DEFINED |
| 0x008ce5b0 | BinaryFindModel | Global::BinaryFindModel | USER_DEFINED |
| 0x008ce5e0 | BinaryInsertModel | Global::BinaryInsertModel | USER_DEFINED |
| 0x008ce680 | InsertModel | Global::InsertModel | USER_DEFINED |
| 0x008ce6b0 | FindModel | Global::FindModel | USER_DEFINED |
| 0x008ce7f0 | LoadModel | Global::LoadModel_2 | USER_DEFINED |
| 0x008ce920 | List<AnimationEvent>::Insert | List_AnimationEvent_::Insert | USER_DEFINED |
| 0x008cee30 | Parse | Global::Parse_13 | USER_DEFINED |
| 0x008cf150 | Parse | Global::Parse_14 | USER_DEFINED |
| 0x008cf300 | Parse | Global::Parse_15 | USER_DEFINED |
| 0x008cf490 | Parse | Global::Parse_16 | USER_DEFINED |
| 0x008cf630 | Parse | Global::Parse_17 | USER_DEFINED |
| 0x008cf7e0 | AnimationParseEvent | Global::AnimationParseEvent | USER_DEFINED |
| 0x008cfbd0 | AurGUIStartLayer | Global::AurGUIStartLayer | USER_DEFINED |
| 0x008cfd00 | AurGUIStopLayer | Global::AurGUIStopLayer | USER_DEFINED |
| 0x008cfd70 | AurGUISetupViewport | Global::AurGUISetupViewport | USER_DEFINED |
| 0x008d02d0 | AurGUICloseViewport | Global::AurGUICloseViewport | USER_DEFINED |
| 0x008d03c0 | GetResolutionTextScalar | Global::GetResolutionTextScalar | USER_DEFINED |
| 0x008d0b30 | CAurGUIStringInternal::~CAurGUIStringInternal | CAurGUIStringInternal::Destructor_2 | USER_DEFINED |
| 0x008d0f90 | CAurGUIStringInternal::GetPositionBoundingRect | CAurGUIStringInternal::GetPositionBoundingRect_2 | USER_DEFINED |
| 0x008d0ff0 | CAurGUIStringInternal::SetPositionBoundingRect | CAurGUIStringInternal::SetPositionBoundingRect_2 | USER_DEFINED |
| 0x008d1050 | CAurGUIStringInternal::GetSizeBoundingRect | CAurGUIStringInternal::GetSizeBoundingRect_2 | USER_DEFINED |
| 0x008d10b0 | CAurGUIStringInternal::SetSizeBoundingRect | CAurGUIStringInternal::SetSizeBoundingRect_2 | USER_DEFINED |
| 0x008d1e10 | CAurGUIStringInternal::Draw | CAurGUIStringInternal::Draw_2 | USER_DEFINED |
| 0x008d38b0 | CAurGUIImageInternal::Draw | CAurGUIImageInternal::Draw_2 | USER_DEFINED |
| 0x008d3980 | CAurGUIImageInternal::CAurGUIImageInternal | CAurGUIImageInternal::Constructor_2 | USER_DEFINED |
| 0x008d3a40 | CAurGUIImageInternal::~CAurGUIImageInternal | CAurGUIImageInternal::Destructor_2 | USER_DEFINED |
| 0x008d3b20 | CAurGUIImageInternal::Draw | CAurGUIImageInternal::Draw_3 | USER_DEFINED |
| 0x008d3c00 | CAurGUIImageInternal::Draw | CAurGUIImageInternal::Draw_4 | USER_DEFINED |
| 0x008d4ca0 | CAurGUIImageInternal::DrawBuffered | CAurGUIImageInternal::DrawBuffered_2 | USER_DEFINED |
| 0x008d5470 | NewCAurGUIImage | Global::NewCAurGUIImage | USER_DEFINED |
| 0x008d5540 | NewCAurGUIString | Global::NewCAurGUIString | USER_DEFINED |
| 0x008d5740 | PartTrigger_scalar_deleting_destructor | Global::PartTrigger_scalar_deleting_destructor | USER_DEFINED |
| 0x008d5860 | AurPartTriggerCreate | Global::AurPartTriggerCreate | USER_DEFINED |
| 0x008d5e30 | AurPartTriggerDelete | Global::AurPartTriggerDelete | USER_DEFINED |
| 0x008d5eb0 | AurPartTriggerSetHighlight | Global::AurPartTriggerSetHighlight | USER_DEFINED |
| 0x008d5ed0 | AurPartTriggerSetHighlightParams | Global::AurPartTriggerSetHighlightParams | USER_DEFINED |
| 0x008d6940 | ImageFlipY | Global::ImageFlipY | USER_DEFINED |
| 0x008d6a10 | ImageSwapRedBlue | Global::ImageSwapRedBlue | USER_DEFINED |
| 0x008d6a70 | ImageDuplicateImageData | Global::ImageDuplicateImageData | USER_DEFINED |
| 0x008d6ae0 | ImageScale | Global::ImageScale | USER_DEFINED |
| 0x008d6de0 | ImageDownsampleRGBA | Global::ImageDownsampleRGBA | USER_DEFINED |
| 0x008d7030 | ImageHalfScale | Global::ImageHalfScale | USER_DEFINED |
| 0x008d7410 | ImageExtractS3TCMipmap | Global::ImageExtractS3TCMipmap | USER_DEFINED |
| 0x008d7500 | AurImageApplyAlpha | Global::AurImageApplyAlpha | USER_DEFINED |
| 0x008d75c0 | AurImageApplyPalette | Global::AurImageApplyPalette | USER_DEFINED |
| 0x008d7750 | ImageWriteTGA | Global::ImageWriteTGA | USER_DEFINED |
| 0x008d7860 | ImageRead16bit | Global::ImageRead16bit | USER_DEFINED |
| 0x008d79c0 | ImageRead16bitHeader | Global::ImageRead16bitHeader | USER_DEFINED |
| 0x008d7ad0 | ImageReadS3TCHeader | Global::ImageReadS3TCHeader | USER_DEFINED |
| 0x008d7b80 | ImageReadS3TCHeader | Global::ImageReadS3TCHeader_2 | USER_DEFINED |
| 0x008d7c80 | ImageReadS3TC | Global::ImageReadS3TC | USER_DEFINED |
| 0x008d7e00 | ImageReadS3TC | Global::ImageReadS3TC_2 | USER_DEFINED |
| 0x008d7ee0 | ImageGetS3TCSize | Global::ImageGetS3TCSize | USER_DEFINED |
| 0x008d7f70 | ImageReadTGAHeader | Global::ImageReadTGAHeader | USER_DEFINED |
| 0x008d7fc0 | ImageReadTGAData | Global::ImageReadTGAData | USER_DEFINED |
| 0x008d8010 | ImageReadTGA | Global::ImageReadTGA | USER_DEFINED |
| 0x008d8130 | ImageRead | Global::ImageRead | USER_DEFINED |
| 0x008d8170 | ImageReadHeader | Global::ImageReadHeader | USER_DEFINED |
| 0x008d8260 | ImageReadProcessed | Global::ImageReadProcessed | USER_DEFINED |
| 0x008d8280 | ImageReadProcessedHeader | Global::ImageReadProcessedHeader | USER_DEFINED |
| 0x008d8290 | ImageReadPLT | Global::ImageReadPLT | USER_DEFINED |
| 0x008d8360 | ImageInterpolate | Global::ImageInterpolate | USER_DEFINED |
| 0x008d8480 | ImageBuildPerlin | Global::ImageBuildPerlin | USER_DEFINED |
| 0x008d8510 | ImageBuildParameterPerlin | Global::ImageBuildParameterPerlin | USER_DEFINED |
| 0x008d85b0 | ImageGetPerlinPixel | Global::ImageGetPerlinPixel | USER_DEFINED |
| 0x008d8650 | ImageGetAlphaMean | Global::ImageGetAlphaMean | USER_DEFINED |
| 0x008d86c0 | ImageLowpassLowQualityBlur | Global::ImageLowpassLowQualityBlur | USER_DEFINED |
| 0x008d8820 | ImageLowpassLowQualityBlur2 | Global::ImageLowpassLowQualityBlur2 | USER_DEFINED |
| 0x008d8a80 | ImageLowpassBlur | Global::ImageLowpassBlur | USER_DEFINED |
| 0x008d8c90 | ImageGetSubImage | Global::ImageGetSubImage | USER_DEFINED |
| 0x008d8d40 | AurImageApplyLowEndEnvMap | Global::AurImageApplyLowEndEnvMap | USER_DEFINED |
| 0x008d9000 | TextureController_scalar_deleting_destructor | Global::TextureController_scalar_deleting_destructor | USER_DEFINED |
| 0x008da470 | WaterTextureController_scalar_deleting_destructor | Global::WaterTextureController_scalar_deleting_destructor | USER_DEFINED |
| 0x008db3f0 | WaveTextureController_scalar_deleting_destructor | Global::WaveTextureController_scalar_deleting_destructor | USER_DEFINED |
| 0x008dc080 | ArturoTextureController_scalar_deleting_destructor | Global::ArturoTextureController_scalar_deleting_destructor | USER_DEFINED |
| 0x008dcbf0 | CycleTIDTextureController_scalar_deleting_destructor | Global::CycleTIDTextureController_scalar_deleting_destructor | USER_DEFINED |
| 0x008dcf50 | DirtyTextureController::Init | DirtyTextureController::Init_2 | USER_DEFINED |
| 0x008dd980 | AurPixelShadersAvailable | Global::AurPixelShadersAvailable | USER_DEFINED |
| 0x008dda10 | AurIsAAliasingAvailable | Global::AurIsAAliasingAvailable | USER_DEFINED |
| 0x008dda60 | glProgramStringARB | Global::glProgramStringARB | USER_DEFINED |
| 0x008dda80 | sInitializeARBFragmentShaders | Global::sInitializeARBFragmentShaders | USER_DEFINED |
| 0x008e7810 | AurGetVertexProgramARBError | Global::AurGetVertexProgramARBError | USER_DEFINED |
| 0x008e7e10 | VertexPrimitive_scalar_deleting_destructor | Global::VertexPrimitive_scalar_deleting_destructor | USER_DEFINED |
| 0x008e7ee0 | VertexPrimitiveFlat_scalar_deleting_destructor | Global::VertexPrimitiveFlat_scalar_deleting_destructor | USER_DEFINED |
| 0x008e7f30 | VertexPrimitiveSkinned_scalar_deleting_destructor | Global::VertexPrimitiveSkinned_scalar_deleting_destructor | USER_DEFINED |
| 0x008e7fd0 | VertexPrimitiveDangly_scalar_deleting_destructor | Global::VertexPrimitiveDangly_scalar_deleting_destructor | USER_DEFINED |
| 0x008e80a0 | VertexPrimitiveAnim_scalar_deleting_destructor | Global::VertexPrimitiveAnim_scalar_deleting_destructor | USER_DEFINED |
| 0x008e8c50 | AurPrimitiveSetupColorClientState | Global::AurPrimitiveSetupColorClientState | USER_DEFINED |
| 0x008e8cb0 | List<Vector>::operator= | List_Vector_::operator= | USER_DEFINED |
| 0x008f4950 | LoadTexture | Global::LoadTexture | USER_DEFINED |
| 0x008f5230 | Material::ReplaceTexture | Material::ReplaceTexture_2 | USER_DEFINED |
| 0x008f5760 | Material::HasEnvMap | Material::HasEnvMap_2 | USER_DEFINED |
| 0x008f5830 | CustomAnimation::~CustomAnimation | CustomAnimation::Destructor_2 | USER_DEFINED |
| 0x008f58a0 | Animation::~Animation | Animation::Destructor_2 | USER_DEFINED |
| 0x008f5b50 | LookAtAnimation::~LookAtAnimation | LookAtAnimation::Destructor_2 | USER_DEFINED |
| 0x008f6ea0 | AnimationParseField | Global::AnimationParseField | USER_DEFINED |
| 0x008f70e0 | FindAnimation | Global::FindAnimation | USER_DEFINED |
| 0x008f7510 | AnimateEvents | Global::AnimateEvents | USER_DEFINED |
| 0x008f9cd0 | ProceduralAnimateHierarchy | Global::ProceduralAnimateHierarchy | USER_DEFINED |
| 0x008fa000 | AnimateHierarchy | Global::AnimateHierarchy | USER_DEFINED |
| 0x008fa9ee | Scene::GetWindVector | Scene::GetWindVector_2 | USER_DEFINED |
| 0x008fbf66 | Gob::Animate | Gob::Animate_2 | USER_DEFINED |
| 0x008fbf99 | Gob::Animate | Gob::Animate_3 | USER_DEFINED |
| 0x008fcb4c | Gob::Animate | Gob::Animate_4 | USER_DEFINED |
| 0x008fcf07 | Gob::Animate | Gob::Animate_5 | USER_DEFINED |
| 0x008fcfb7 | Gob::Animate | Gob::Animate_6 | USER_DEFINED |
| 0x008fd2f0 | GetCurrentRun | Global::GetCurrentRun | USER_DEFINED |
| 0x008fd360 | GetAnimationModelScale | Global::GetAnimationModelScale | USER_DEFINED |
| 0x008fdd20 | List<T*>::Insert | List_T__::Insert | USER_DEFINED |
| 0x008fdd80 | AurInternalPrepareATIShaders | Global::AurInternalPrepareATIShaders | USER_DEFINED |
| 0x008fe280 | AurVertexProgramMakeCurrent | Global::AurVertexProgramMakeCurrent | USER_DEFINED |
| 0x008fe2a0 | List<SafePointer<WindManager>*>::~List | List_SafePointer_WindManager___::Destructor | USER_DEFINED |
| 0x008fe340 | PointSourceWind::PointSourceWind | PointSourceWind::Constructor_2 | USER_DEFINED |
| 0x008ff5f0 | SafePointerList<WindManager>::~SafePointerList | SafePointerList_WindManager_::Destructor | USER_DEFINED |
| 0x008ff680 | List<PointSourceWind>::List | List_PointSourceWind_::List | USER_DEFINED |
| 0x008ff6c0 | List<PointSourceWind>::~List | List_PointSourceWind_::Destructor | USER_DEFINED |
| 0x008ff700 | List<PointSourceWind>::Add | List_PointSourceWind_::Add | USER_DEFINED |
| 0x008ff7d0 | List<PointSourceWind>::RemoveAt | List_PointSourceWind_::RemoveAt | USER_DEFINED |
| 0x008ff8c0 | List<SafePointer<WindManager>*>::allocate | List_SafePointer_WindManager___::allocate | USER_DEFINED |
| 0x008ff920 | List<PointSourceWind>::allocate | List_PointSourceWind_::allocate | USER_DEFINED |
| 0x008ffa80 | lightcmp | Global::lightcmp | USER_DEFINED |
| 0x008ffc70 | SetLightGL | Global::SetLightGL | USER_DEFINED |
| 0x00903780 | LightManager_VP_camera_basis_helper | Global::LightManager_VP_camera_basis_helper | USER_DEFINED |
| 0x00903a00 | LightManager::SetupLightsForThisPositionAndRadius | LightManager::SetupLightsForThisPositionAndRadius_2 | USER_DEFINED |
| 0x00903ca0 | AurLiteClearTimeTriggers | Global::AurLiteClearTimeTriggers | USER_DEFINED |
| 0x00903f30 | IODispatcher::ReadSync | IODispatcher::ReadSync_2 | USER_DEFINED |
| 0x00903f60 | IODispatcher::ReadSync | IODispatcher::ReadSync_3 | USER_DEFINED |
| 0x009040e0 | IODispatcher::ReadSync | IODispatcher::ReadSync_4 | USER_DEFINED |
| 0x00904480 | anonymous_namespace_Input_InputBinary_ResetMdlNode_MdlNode_ptr_uchar_ptr_void_ptr | Global::anonymous_namespace_Input_InputBinary_ResetMdlNode_MdlNode_ptr_uchar_ptr_void_ptr | USER_DEFINED |
| 0x00904700 | anonymous_namespace_Input_InputBinary_ResetMdlNodeParts_MdlNode_ptr_uchar_ptr_void_ptr | Global::anonymous_namespace_Input_InputBinary_ResetMdlNodeParts_MdlNode_ptr_uchar_ptr_void_ptr | USER_DEFINED |
| 0x009047b0 | anonymous_namespace_Input_InputBinary_ResetMdlNodeChild | Global::anonymous_namespace_Input_InputBinary_ResetMdlNodeChild | USER_DEFINED |
| 0x00904870 | anonymous_namespace_Input_InputBinary_ResetMdlNodePartsChild | Global::anonymous_namespace_Input_InputBinary_ResetMdlNodePartsChild | USER_DEFINED |
| 0x009048e0 | anonymous_namespace_Input_InputBinary_ResetMdlNodeByType | Global::anonymous_namespace_Input_InputBinary_ResetMdlNodeByType | USER_DEFINED |
| 0x00904ad0 | anonymous_namespace_Input_InputBinary_ResetMdlNodePartsByType | Global::anonymous_namespace_Input_InputBinary_ResetMdlNodePartsByType | USER_DEFINED |
| 0x00905246 | AurTextureGetReference | Global::AurTextureGetReference_2 | USER_DEFINED |
| 0x00905280 | InputBinary::ResetTextureReferenceNode | InputBinary::ResetTextureReferenceNode_2 | USER_DEFINED |
| 0x009055d0 | anonymous_namespace_Input_InputBinary_ResetTriMeshParts_MdlNodeTriMesh_ptr_uchar_ptr_uchar_ptr | Global::anonymous_namespace_Input_InputBinary_ResetTriMeshParts_MdlNodeTriMesh_ptr_uchar_ptr_uchar_ptr | USER_DEFINED |
| 0x00905970 | InputBinary::ResetTriMeshPartsOnly | InputBinary::ResetTriMeshPartsOnly_2 | USER_DEFINED |
| 0x00905b80 | anonymous_namespace_Input_InputBinary_ResetAABBTree_AABB_ptr_uchar_ptr_uchar_ptr | Global::anonymous_namespace_Input_InputBinary_ResetAABBTree_AABB_ptr_uchar_ptr_uchar_ptr | USER_DEFINED |
| 0x00907710 | CAuroraModel::~CAuroraModel | CAuroraModel::Destructor_2 | USER_DEFINED |
| 0x00907780 | CAuroraModel::GetPtrToResHelperInfo | CAuroraModel::GetPtrToResHelperInfo_2 | USER_DEFINED |
| 0x00907b60 | CAuroraCompressedTexture::~CAuroraCompressedTexture | CAuroraCompressedTexture::Destructor_2 | USER_DEFINED |
| 0x009098f0 | CAuroraLayeredTexture::~CAuroraLayeredTexture | CAuroraLayeredTexture::Destructor_2 | USER_DEFINED |
| 0x00909de0 | CAuroraTexture::~CAuroraTexture | CAuroraTexture::Destructor_2 | USER_DEFINED |
| 0x0090a490 | CAuroraProcessedTexture::~CAuroraProcessedTexture | CAuroraProcessedTexture::Destructor_2 | USER_DEFINED |
| 0x0090ae00 | CAuroraVisibility::~CAuroraVisibility | CAuroraVisibility::Destructor_2 | USER_DEFINED |
| 0x0090b200 | CAuroraTXI::~CAuroraTXI | CAuroraTXI::Destructor_2 | USER_DEFINED |
| 0x0090b640 | CAuroraModelX::~CAuroraModelX | CAuroraModelX::Destructor_2 | USER_DEFINED |
| 0x0090ba40 | CAurora16BitTexture::~CAurora16BitTexture | CAurora16BitTexture::Destructor_2 | USER_DEFINED |
| 0x0090bfb0 | CResDDS::~CResDDS | CResDDS::Destructor_2 | USER_DEFINED |
| 0x0090c150 | CResPLT::~CResPLT | CResPLT::Destructor_2 | USER_DEFINED |
| 0x0090c170 | CResPLT::GetPLTAttrib | CResPLT::GetPLTAttrib_2 | USER_DEFINED |
| 0x0090c380 | CResPLT::ReadRLEPLT | CResPLT::ReadRLEPLT_2 | USER_DEFINED |
| 0x0090c530 | CResVIS::~CResVIS | CResVIS::Destructor_2 | USER_DEFINED |
| 0x0090c5d0 | CResTXI::~CResTXI | CResTXI::Destructor_2 | USER_DEFINED |
| 0x0090c670 | CResMDX::~CResMDX | CResMDX::Destructor_2 | USER_DEFINED |
| 0x0090c720 | CRes4PC::~CRes4PC | CRes4PC::Destructor_2 | USER_DEFINED |
| 0x0090deb0 | InitControl::ScaleToEdgeHelper | InitControl::ScaleToEdgeHelper_2 | USER_DEFINED |
| 0x0090df00 | InitControl::ScaleToEdgeHelper | InitControl::ScaleToEdgeHelper_3 | USER_DEFINED |
| 0x0090dfc0 | InitControl::ScaleToEdgeHelper | InitControl::ScaleToEdgeHelper_4 | USER_DEFINED |
| 0x0090e290 | CSWGuiPanel::SendControlToBack | CSWGuiPanel::SendControlToBack_2 | USER_DEFINED |
| 0x00911ec0 | CSWGuiManager::HideSoftwareMouse | CSWGuiManager::HideSoftwareMouse_2 | USER_DEFINED |
| 0x00913030 | CSWGuiBorder::~CSWGuiBorder | CSWGuiBorder::Destructor_2 | USER_DEFINED |
| 0x009147e0 | CSWGuiImage::~CSWGuiImage | CSWGuiImage::Destructor_2 | USER_DEFINED |
| 0x00914e70 | CSWGuiTextParams::CSWGuiTextParams | CSWGuiTextParams::Constructor_2 | USER_DEFINED |
| 0x00915550 | CSWGuiTextParams::SetDefaultColor | CSWGuiTextParams::SetDefaultColor_2 | USER_DEFINED |
| 0x00915660 | CSWGuiText::~CSWGuiText | CSWGuiText::Destructor_2 | USER_DEFINED |
| 0x009164f0 | CSWGuiScene::~CSWGuiScene | CSWGuiScene::Destructor_2 | USER_DEFINED |
| 0x00916770 | CSWGuiScene::AddModel | CSWGuiScene::AddModel_2 | USER_DEFINED |
| 0x00917090 | CSWGuiControl::Load | CSWGuiControl::Load_2 | USER_DEFINED |
| 0x009171c0 | CSWGuiControl::Load | CSWGuiControl::Load_3 | USER_DEFINED |
| 0x009179b0 | CSWGuiManager::RemoveToolTip | CSWGuiManager::RemoveToolTip_2 | USER_DEFINED |
| 0x00917ae0 | CSWGuiLabel::Initialize | CSWGuiLabel::Initialize_2 | USER_DEFINED |
| 0x00917e10 | CSWGuiLabel::Draw | CSWGuiLabel::Draw_2 | USER_DEFINED |
| 0x00918130 | CSWGuiButton::Initialize | CSWGuiButton::Initialize_2 | USER_DEFINED |
| 0x00918ab0 | CSWGuiButtonToggle::Initialize | CSWGuiButtonToggle::Initialize_2 | USER_DEFINED |
| 0x00918e00 | CSWGuiSlider::Initialize | CSWGuiSlider::Initialize_2 | USER_DEFINED |
| 0x009199b0 | CSWGuiProgressBar::Initialize | CSWGuiProgressBar::Initialize_2 | USER_DEFINED |
| 0x0091a0d0 | CSWGuiScrollBar::~CSWGuiScrollBar | CSWGuiScrollBar::Destructor_2 | USER_DEFINED |
| 0x0091ab20 | CSWGuiScrollBar::Initialize | CSWGuiScrollBar::Initialize_2 | USER_DEFINED |
| 0x0091b1a0 | CSWGuiListBox::~CSWGuiListBox | CSWGuiListBox::Destructor_2 | USER_DEFINED |
| 0x0091b530 | CSWGuiListBox::Initialize | CSWGuiListBox::Initialize_2 | USER_DEFINED |
| 0x0091dd30 | CSWGuiListBox::AddControls | CSWGuiListBox::AddControls_2 | USER_DEFINED |
| 0x0091de80 | CSWGuiListBox::SetActiveControl | CSWGuiListBox::SetActiveControl_2 | USER_DEFINED |
| 0x0091f4b0 | CExoArrayList<CSWGuiExtent>::CExoArrayList<CSWGuiExtent> | CExoArrayList_CSWGuiExtent_::Constructor | USER_DEFINED |
| 0x0091f690 | ExoMoviePlayerWindowProc | Global::ExoMoviePlayerWindowProc | USER_DEFINED |
| 0x00920620 | ExoMoviePlayerThreadProc | Global::ExoMoviePlayerThreadProc | USER_DEFINED |
| 0x00921295 | sprintf | Global::sprintf | USER_DEFINED |
| 0x00921347 | operator_new_uint | Global::operator_new_uint | USER_DEFINED |
| 0x00921b40 | strlen | Global::strlen | USER_DEFINED |
| 0x00924260 | memset | Global::memset | USER_DEFINED |
| 0x009242e5 | free | Global::free | USER_DEFINED |
| 0x009243dd | sscanf | Global::sscanf | USER_DEFINED |
| 0x00924754 | sqrt | Global::sqrt | USER_DEFINED |
| 0x00924b48 | atoi | Global::atoi | USER_DEFINED |
| 0x00924c20 | strcpy | Global::strcpy | USER_DEFINED |
| 0x00924eb8 | new | Global::new | USER_DEFINED |
| 0x00924fea | atexit | Global::atexit | USER_DEFINED |
| 0x009251a8 | entry | Global::entry | IMPORTED |
| 0x00925530 | memcpy | Global::memcpy | USER_DEFINED |
| 0x00925936 | rand | Global::rand | USER_DEFINED |
| 0x009259f0 | sin | Global::sin | USER_DEFINED |
| 0x00925b20 | cos | Global::cos | USER_DEFINED |
| 0x00925d10 | acos | Global::acos | USER_DEFINED |
| 0x00926f00 | log | Global::log | USER_DEFINED |
| 0x00927110 | strchr | Global::strchr | USER_DEFINED |
| 0x00927220 | pow | Global::pow | USER_DEFINED |
| 0x00927760 | tan | Global::tan | USER_DEFINED |
| 0x009279d5 | printf | Global::printf | USER_DEFINED |
| 0x00927ae0 | atan | Global::atan | USER_DEFINED |
| 0x009293cc | int_toupper_int | Global::int_toupper_int | USER_DEFINED |
| 0x00929e30 | asin | Global::asin | USER_DEFINED |
| 0x00929f80 | ceilf | Global::ceilf | USER_DEFINED |
| 0x0093378c | CExoResMan::Dump | CExoResMan::Dump_2 | USER_DEFINED |
| 0x0093e9ce | strncmp | Global::strncmp | USER_DEFINED |
| 0x009434c0 | CResWave::~CResWave | CResWave::Destructor_2 | USER_DEFINED |
| 0x009439d0 | CResTGA::~CResTGA | CResTGA::Destructor_2 | USER_DEFINED |
| 0x009443e0 | CResTPC::~CResTPC | CResTPC::Destructor_2 | USER_DEFINED |
| 0x00944a50 | BSPTraverse | Global::BSPTraverse | USER_DEFINED |
| 0x00944cf0 | BSPTraverseVolumeApprox | Global::BSPTraverseVolumeApprox | USER_DEFINED |
| 0x00945180 | BSPFitSphere | Global::BSPFitSphere | USER_DEFINED |
| 0x00945210 | BSPFitBox | Global::BSPFitBox | USER_DEFINED |
| 0x00945320 | BSPSphereCheck | Global::BSPSphereCheck | USER_DEFINED |
| 0x00945400 | BSPSearch | Global::BSPSearch | USER_DEFINED |
| 0x009454f0 | BSPHitCheck | Global::BSPHitCheck | USER_DEFINED |
| 0x00983330 | static_init_CSWCAmbientSound_THUNDER_ROLLING1 | Global::static_init_CSWCAmbientSound_THUNDER_ROLLING1 | USER_DEFINED |
| 0x00983360 | static_init_CSWCAmbientSound_THUNDER_ROLLING2 | Global::static_init_CSWCAmbientSound_THUNDER_ROLLING2 | USER_DEFINED |
| 0x00983390 | static_init_CSWCAmbientSound_THUNDER1 | Global::static_init_CSWCAmbientSound_THUNDER1 | USER_DEFINED |
| 0x009833c0 | static_init_CSWCAmbientSound_THUNDER2 | Global::static_init_CSWCAmbientSound_THUNDER2 | USER_DEFINED |
| 0x00983e80 | aurlite_global_initializer_for_timetriggers | Global::aurlite_global_initializer_for_timetriggers | USER_DEFINED |
| 0x00984970 | aurlite_timetriggers_atexit_destructor_wrapper | Global::aurlite_timetriggers_atexit_destructor_wrapper | USER_DEFINED |

## (c) Per-program counts

| program | functions | by source | symbols (all) | by source | trusted fn | trusted fn with namespace |
|---|---|---|---|---|---|---|
| k2_win_gog_aspyr_swkotor2.exe | 26975 | ANALYSIS 15172, DEFAULT 1380, IMPORTED 1, USER_DEFINED 10422 | 114099 | ANALYSIS 17110, DEFAULT 93447, IMPORTED 2284, USER_DEFINED 1258 | 10423 | 9094 |
| k2_win_steam_aspyr_swkotor2.exe | 26965 | ANALYSIS 15295, DEFAULT 11645, IMPORTED 1, USER_DEFINED 24 | 114497 | ANALYSIS 16869, DEFAULT 94401, IMPORTED 2944, USER_DEFINED 283 | 25 | 1 |
| k2_linux_swkotor2.elf | 15630 | ANALYSIS 85, DEFAULT 10187, IMPORTED 406, USER_DEFINED 4952 | 165899 | ANALYSIS 14447, DEFAULT 151273, IMPORTED 143, USER_DEFINED 36 | 5358 | 2 (C++ class-method look) |

