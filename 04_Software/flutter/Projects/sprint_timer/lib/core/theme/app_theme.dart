import 'package:flutter/material.dart';

class AppColors {
  // Backgrounds
  static const bg = Color(0xFF0A0C10);
  static const surface = Color(0xFF1A1E28);
  static const surfaceHigh = Color(0xFF242938);

  // Accents semanticos
  static const red = Color(0xFFFF4C6A);        // HR / frecuencia cardíaca
  static const violet = Color(0xFFA78BFA);     // ECV / eficiencia cardiovascular
  static const green = Color(0xFF34D399);      // PR, estado OK, mejora
  static const amber = Color(0xFFFBBF24);      // Advertencia, recuperación
  // Texto
  static const textPrimary = Color(0xFFE8EAF0);
  static const textSecondary = Color(0xFF8892AA);
  static const textTertiary = Color(0xFF5A6080);

  // Brand Acents
  static const Color cyan = Color(0xFF66FCF1); // Primary neon
  static const Color cyanDim = Color(0xFF45A29E);

  // Borders
  static const border = Color(0xFF252A3A);
  static const borderMid = Color(0xFF2E3448);
}

class AppTheme {
  static ThemeData get dark => ThemeData(
    useMaterial3: true,
    brightness: Brightness.dark,
    scaffoldBackgroundColor: AppColors.bg,
    colorScheme: const ColorScheme.dark(
      primary: AppColors.cyan,
      secondary: AppColors.violet,
      tertiary: AppColors.green,
      error: AppColors.red,
      surface: AppColors.surface,
      onSurface: AppColors.textPrimary,
      onPrimary: AppColors.bg,
    ),
    fontFamily: 'DMSans',
    textTheme: const TextTheme(
      displayLarge: TextStyle(fontSize: 32, fontWeight: FontWeight.w700, color: AppColors.textPrimary),
      headlineMedium: TextStyle(fontSize: 20, fontWeight: FontWeight.w600, color: AppColors.textPrimary),
      titleMedium: TextStyle(fontSize: 16, fontWeight: FontWeight.w600, color: AppColors.textPrimary),
      titleSmall: TextStyle(fontSize: 14, fontWeight: FontWeight.w500, color: AppColors.textSecondary),
      bodyMedium: TextStyle(fontSize: 14, fontWeight: FontWeight.w400, color: AppColors.textPrimary),
      bodySmall: TextStyle(fontSize: 12, fontWeight: FontWeight.w400, color: AppColors.textSecondary),
      labelSmall: TextStyle(fontSize: 11, fontWeight: FontWeight.w500, color: AppColors.textTertiary),
    ),
    appBarTheme: const AppBarTheme(
      backgroundColor: AppColors.bg,
      foregroundColor: AppColors.textPrimary,
      elevation: 0,
      centerTitle: false,
    ),
    navigationBarTheme: NavigationBarThemeData(
      backgroundColor: AppColors.surface,
      indicatorColor: AppColors.cyan.withValues(alpha: 0.15),
      iconTheme: WidgetStateProperty.resolveWith((states) {
        if (states.contains(WidgetState.selected)) {
          return const IconThemeData(color: AppColors.cyan, size: 22);
        }
        return const IconThemeData(color: AppColors.textTertiary, size: 22);
      }),
      labelTextStyle: WidgetStateProperty.resolveWith((states) {
        if (states.contains(WidgetState.selected)) {
          return const TextStyle(color: AppColors.cyan, fontSize: 11, fontWeight: FontWeight.w600, fontFamily: 'DMSans');
        }
        return const TextStyle(color: AppColors.textTertiary, fontSize: 11, fontFamily: 'DMSans');
      }),
    ),
    cardTheme: CardThemeData(
      color: AppColors.surface,
      elevation: 0,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
        side: const BorderSide(color: AppColors.border, width: 0.5),
      ),
      margin: EdgeInsets.zero,
    ),
    inputDecorationTheme: InputDecorationTheme(
      filled: true,
      fillColor: AppColors.surface,
      border: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: const BorderSide(color: AppColors.border, width: 0.5),
      ),
      enabledBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: const BorderSide(color: AppColors.border, width: 0.5),
      ),
      focusedBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: const BorderSide(color: AppColors.cyan, width: 1),
      ),
      labelStyle: const TextStyle(color: AppColors.textSecondary, fontSize: 14),
      hintStyle: const TextStyle(color: AppColors.textTertiary, fontSize: 14),
    ),
    dividerTheme: const DividerThemeData(color: AppColors.border, thickness: 0.5),
    chipTheme: ChipThemeData(
      backgroundColor: AppColors.surfaceHigh,
      labelStyle: const TextStyle(fontSize: 12, fontFamily: 'DMSans'),
      side: const BorderSide(color: AppColors.border, width: 0.5),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
      padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
    ),
  );
}

// Constantes monoespaciadas para el tiempo
const monoStyle = TextStyle(
  fontFamily: 'JetBrainsMono',
  fontVariations: [],
);
